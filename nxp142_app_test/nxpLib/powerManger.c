#include "stdint.h"
#include "usertype.h"
#include "stdlib.h"
#include "string.h"
#include "powerManger.h"
#include "device_registers.h" 

/*******************************************************************************
Function: clkout_bus
*******************************************************************************/
void clkout_bus(void)
{
    SIM->CHIPCTL &= ~(1 << 11); //Disable CLKOUT
    SIM->CHIPCTL |= (9 << 4);   //BUS_CLK
    SIM->CHIPCTL |= (1 << 11);  //Enable CLKOUT
    // [11] CLKOUTEN = 1 CLKOUT enable
    // [7-4] CLKOUTSEL = 0b1001 BUS_CLK
}

/*******************************************************************************
Function: init_SIRC
Notes   : SIRC 8MHz, SIRCDIV1 1MHz, SIRCDIV2 1MHz
        : SIRC disabled in VLPS
*******************************************************************************/
void init_SIRC(void)
{
    SCG->SIRCCSR &= ~ (1 << 24);
    // [24] LK = 0 Unlock Control Status Register

    SCG->SIRCCSR |= 0x01;
    // [2] SIRCLPEN = 0 Slow IRC is disabled in VLP modes
    // [1] SIRCSTEN = 0 Slow IRC is disabled in Stop modes
    // [0] SIRCEN = 1 Slow IRC is enabled

    SCG->SIRCDIV |= 0x0404;
    // [10-8] SIRCDIV2 0b100 Divide by 8 (1MHz)
    // [2-0]  SIRCDIV1 0b100 Divide by 8 (1MHz)

    while((SCG->SIRCCSR & (1 << 24)) == 0); // wait until clock is valid
    // [24] SIRCVLD = 1 Slow IRC is enabled and output clock is valid

    SCG->SIRCCSR |= (1 << 24);
    // [24] LK = 1 lock Control Status Register

}

/*******************************************************************************
Function: init_VLPR
Notes   : Max frequencies in VLPR: Core/System 4MHz, Bus: 2MHz, Flash: 1MHz
        : See e11063 mask 0N57U
*******************************************************************************/
void init_VLPR(void)
{
    SCG->VCCR = 0x02010013;
    // [27-24] SCS = 2     Slow IRC (SIRC_CLK 8MHZ)
    // [19-16] DIVCORE = 1 Divide by 2  (4 MHz)
    // [7-4] DIVBUS = 1    Divide core by 2  (2 MHz)
    // [3-1] DIVSLOW = 3   Divide core by 4  (1 MHz)
}

/*******************************************************************************
Function: switch_to_SIRC_in_RUN
*******************************************************************************/
void switch_to_SIRC_in_RUN(void)
{
    uint32_t srie = RCM->SRIE;
    RCM->SRIE = 0x0000; // configure all reset sources to be ?eset' (not as Interrupt)
    RCM->SRIE = 0xFFFF; // Program each reset source as Interrupt via RCM_SRIE
    // for a minimum delay time of 10 LPO.

    SCG->RCCR = 0x02010013;
    // [27-24] SCS = 2  Slow IRC (SIRC_CLK 8MHZ)
    // [19-16] DIVCORE = 1 Divide by 2  (4 MHz)
    // [7-4] DIVBUS = 1     Divide core by 2  (2 MHz)
    // [3-1] DIVSLOW = 3   Divide core by 4  (1 MHz)

    while(!((SCG->CSR & (0x0F000000)) & 0x02000000));
    // [27-24] SCS = 0b0010 Slow IRC (SIRC_CLK)
    // or
    while((SCG->SIRCCSR & (1 << 25)) == 0);
    // [25] SIRCCSR = 1 Until SIRC is the system clock source

    RCM->SRIE = srie;
}

/*******************************************************************************
Function: disable_FIRC_in_RUN
*******************************************************************************/
void disable_FIRC_in_RUN(void)
{
    // When entering VLPR/VLPS mode, the system clock should be SIRC. The FIRC, SOSC,
    // and SPLL must be disabled by software in RUN mode before making any mode
    // transition.

    if(!(SCG->FIRCCSR & (1 << 25)))
    {   // [25] FIRCSEL, if FIRC is not the system clock source
        SCG->FIRCCSR &= ~(1 << 0);
        // [0] FIRCEN = 0 FIRC disabled
        while(SCG->FIRCCSR & (1 << 24));
        // [24] FIRCVLD = 0 Fast IRC is not enabled or clock is not valid
    }
}

/*******************************************************************************
Function: enter_VLPR
*******************************************************************************/
void enter_VLPR(void)
{
    init_SIRC();
    switch_to_SIRC_in_RUN();
    disable_FIRC_in_RUN();
    init_VLPR();

    SMC->PMPROT |= (1 << 5);
    // [5] AVLP = 1 VLPR and VLPS allowed

    PMC->REGSC |= (1 << 0);
    // [0] BIASEN = 1 Biasing enabled

    if(SMC->PMSTAT == 0x01) // [7-0] PMSTAT: 0x01 from RUN
    {
        SMC->PMCTRL |= 0x00000040; // enter VLPR
        // [6-5] RUNM = 0b10 Very-Low-Power Run mode (VLPR)

        while(!((SMC->PMSTAT & 0xFF) & 0x4))
        {   // [7:0] PMSTAT = 0x04 VLPR
            // wait until the current power mode is VLPR
        }
    }
}

/*******************************************************************************
Function: enter_VLPS
Notes   : VLPS in Sleep-On-Exit mode
        : Should VLPS transition failed, reset the MCU
*******************************************************************************/
void enter_VLPS(void)
{
//		if(SMC->PMSTAT == 0x01){
//			init_SIRC();
//			switch_to_SIRC_in_RUN();
//			disable_FIRC_in_RUN();
//		}
    if(SMC->PMSTAT == 0x01)
    { // [7-0] PMSTAT: 0x04 from VLPR
			//------------------------------------------------
			/* Disable Clock monitor for System Oscillator */
			SCG->SOSCCSR &= ~(SCG_SOSCCSR_SOSCCM_MASK);
			/* Disable Clock monitor for System PLL */
			SCG->SPLLCSR &= ~(SCG_SPLLCSR_SPLLCM_MASK);
			//-------------------------------------------------
			uint32_t tempSIRC = SCG->SIRCCSR;
			/* Disable in VLPS */
			tempSIRC &= ~(SCG_SIRCCSR_SIRCLPEN_MASK |
									SCG_SIRCCSR_SIRCSTEN_MASK);
			SCG->SIRCCSR = tempSIRC;
			//-------------------------------------------------
//        S32_SCB->SCR |= 0x04;//0x06;
//        // [4] SEVONPEND = 0 only enabled interrupts can wakeup
//        // [2] SLEEPDEEP = 1 deep sleep
//        // [1] SLEEPONEXIT = 1 enter sleep, or deep sleep, on return from an ISR

//        PMC->REGSC |= (1 << 1);
//        // [1] CLKBIASDIS = 1 In VLPS mode, the bias current for SIRC, FIRC, PLL is disabled

//        SMC->PMCTRL |= 0x00000002;
//        // [2-0] STOPM = Very-Low-Power Stop (VLPS)

//        SMC->PMPROT |= (1 << 5);
        // [5] AVLP = 1 VLPS allowed
				PMC->REGSC |= PMC_REGSC_BIASEN_MASK
				#if (0 == SCG_ENABLE_SIRC_IN_VLPS)
										| PMC_REGSC_CLKBIASDIS_MASK
				#endif
										;

				/* Enable Stop Modes in the Core */
				S32_SCB->SCR |= S32_SCB_SCR_SLEEPDEEP_MASK;

				/*  Select VLPS Mode */
				SMC->PMCTRL = SMC_PMCTRL_STOPM(0b10);
        (void)SMC->PMPROT; // Read-After-Write to ensure the register is written
				__asm("DSB");
        __asm("ISB");
        STANDBY();  // Move to Stop mode

        if (SMC->PMCTRL & 0x8)
        { // [3] VLPSA The previous stop mode entry was aborted

            // If aborted
            STANDBY();  // Move to Stop mode

            if (SMC->PMCTRL & 0x8)
            { // [3] VLPSA The previous stop mode entry was aborted

                // If aborted, reset the MCU.
                S32_SCB->AIRCR = 0x05FA0004;
                // [31:16] VECTKEY = 05FA Write 0x5FA to VECTKEY, otherwise the write is ignored
                // [2] SYSRESETREQ = 1 Asserts a signal to the outer system that requests a reset
            }
        }
    }
}

void enter_STOP(int mode)//1--stop1  2--stop2
{			
    if((SMC->PMSTAT == 0x01) && ((mode == 1) || (mode == 2)))
    { // [7-0] PMSTAT: 0x04 from VLPR
				switch_to_SIRC_in_RUN();
			  disable_FIRC_in_RUN();
			
        S32_SCB->SCR |= 0x06;//0x06;
        // [4] SEVONPEND = 0 only enabled interrupts can wakeup
        // [2] SLEEPDEEP = 1 deep sleep
        // [1] SLEEPONEXIT = 1 enter sleep, or deep sleep, on return from an ISR
			  if(mode == 1)
					SMC->STOPCTRL |= 0x00000040;
				else
					SMC->STOPCTRL |= 0x00000080;
        // [2-0] STOPM = Very-Low-Power Stop (VLPS)


        (void)SMC->STOPCTRL; // Read-After-Write to ensure the register is written
        STANDBY();  // Move to Stop mode

        if (SMC->PMCTRL & 0x8)
        { // [3] VLPSA The previous stop mode entry was aborted

            // If aborted
            STANDBY();  // Move to Stop mode

            if (SMC->PMCTRL & 0x8)
            { // [3] VLPSA The previous stop mode entry was aborted

                // If aborted, reset the MCU.
                S32_SCB->AIRCR = 0x05FA0004;
                // [31:16] VECTKEY = 05FA Write 0x5FA to VECTKEY, otherwise the write is ignored
                // [2] SYSRESETREQ = 1 Asserts a signal to the outer system that requests a reset
            }
        }
    }
}
