/*
 * lowPower.c
 *
 *  Created on: Aug 29, 2017
 *      Author: B50982
 */

#include "lowPower.h"
#include "delay.h"
#include "config.h"
#include "clock.h"
#include "clocks_and_modes.h"
#include <stdint.h>

static inline uint8_t FLEXCAN_GetWTOF(void)
{
    return (uint8_t)((CAN0->WU_MTC & CAN_WU_MTC_WTOF_MASK) >> CAN_WU_MTC_WTOF_SHIFT);
}

/*!
 * @brief Clears the Wake Up by Timeout Flag Bit.
 *
 * @param   base  The FlexCAN base address
 */
static inline void FLEXCAN_ClearWTOF(void)
{
    CAN0->WU_MTC |= CAN_WU_MTC_WTOF_MASK;
}

/*!
 * @brief Gets the Wake Up by Match Flag Bit.
 *
 * @param   base  The FlexCAN base address
 * @return  the Wake Up by Match Flag Bit
 */
static inline uint8_t FLEXCAN_GetWUMF(void)
{
    return (uint8_t)((CAN0->WU_MTC & CAN_WU_MTC_WUMF_MASK) >> CAN_WU_MTC_WUMF_SHIFT);
}

/*!
 * @brief Clears the Wake Up by Match Flag Bit.
 *
 * @param   base  The FlexCAN base address
 */
static inline void FLEXCAN_ClearWUMF(void)
{
    CAN0->WU_MTC |= CAN_WU_MTC_WUMF_MASK;
}

void CAN0_Wake_Up_IRQHandler(void)
{
    if (CAN0->MCR & CAN_MCR_PNET_EN_MASK)
		{
				if (FLEXCAN_GetWTOF() != 0U)
				{
						FLEXCAN_ClearWTOF();
				}
				if (FLEXCAN_GetWUMF() != 0U)
				{
						FLEXCAN_ClearWUMF();
				}
		}
}


//CAN0->MCR |= CAN_MCR_PNET_EN_MASK;//20190815
void freeze_config(void){
		int timeout = 0;
		CAN0->MCR |= CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK;
		if(CAN0->MCR & CAN_MCR_MDIS_MASK)
			CAN0->MCR &= (~CAN_MCR_MDIS_MASK);
		while(!(CAN0->MCR & CAN_MCR_FRZACK_MASK) && (timeout++ < 50000));
		if(!(CAN0->MCR & CAN_MCR_FRZACK_MASK)){
			CAN0->MCR |= CAN_MCR_SOFTRST_MASK;
			while(!(CAN0->MCR & CAN_MCR_SOFTRST_MASK));
			
		  CAN0->MCR = 0x0000001F; 
			CAN0->MCR |= CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK;
			if(CAN0->MCR & CAN_MCR_MDIS_MASK)
				CAN0->MCR &= (~CAN_MCR_MDIS_MASK);
			
			CAN0->IMASK1 = 1<<4 | 1<< 3;
		}
		
}

void pnet_config(void){
	//------enter freeze-----------------------------------------------
	freeze_config();
	//------config pnet-----------------------------------------------------------
	CAN0->CTRL1_PN = 0;
	CAN0->CTRL1_PN |= CAN_CTRL1_PN_FCS(0) |  // Message ID filtering only
										CAN_CTRL1_PN_IDFS(1) | 
										CAN_CTRL1_PN_NMATCH(1) |
										CAN_CTRL1_PN_WTOF_MSK(1);
//	CAN0->FLT_ID1 = 0;
//	CAN0->FLT_ID1 |= CAN_FLT_ID1_FLT_IDE(0) |
//										CAN_FLT_ID1_FLT_RTR(0) |
//										CAN_FLT_ID1_FLT_ID1(0X123);
	
	CAN0->FLT_ID2_IDMASK = 0;
	CAN0->FLT_ID2_IDMASK |= CAN_FLT_ID2_IDMASK_FLT_ID2_IDMASK(0X123);
	
	//---------pnet enable-----------------------------------------------------------------------------
	CAN0->MCR |= CAN_MCR_PNET_EN_MASK;//20190815
	S32_NVIC->ISER[(uint32_t)(CAN0_Wake_Up_IRQn) >> 5U] = (uint32_t)(1U << ((uint32_t)(CAN0_Wake_Up_IRQn) & (uint32_t)0x1FU));
	S32_NVIC->ICPR[(uint32_t)(CAN0_Wake_Up_IRQn) >> 5U] = (uint32_t)(1U << ((uint32_t)(CAN0_Wake_Up_IRQn) & (uint32_t)0x1FU));
	S32_NVIC->IP[CAN0_Wake_Up_IRQn] = 0xA;              /* priority 10 of 0-15*/
	
	 //---exit freeze------------------------------------------------------
	 CAN0->MCR = (CAN0->MCR & ~CAN_MCR_HALT_MASK) | CAN_MCR_HALT(0U);
   CAN0->MCR = (CAN0->MCR & ~CAN_MCR_FRZ_MASK) | CAN_MCR_FRZ(0U);
   /* Wait till exit freeze mode */
   while (((CAN0->MCR & CAN_MCR_FRZACK_MASK) >> CAN_MCR_FRZACK_SHIFT) != 0U) {}
  
}
void Run_to_VLPS (void)
{
    /* Disable clock monitors on SCG module */
    disable_clock_monitors();
    /* SCG configuration for VLPS */
    scg_vlps_configuration();
    /* Allow very low power modes*/
    SMC->PMPROT |= SMC_PMPROT_AVLP_MASK;

    /* CLKBIASDIS=1: In VLPS mode, the bias currents and reference voltages
     * for the following clock modules are disabled: SIRC, FIRC, PLL */
    PMC->REGSC |= PMC_REGSC_BIASEN_MASK
#if (0 == SCG_ENABLE_SIRC_IN_VLPS)
            | PMC_REGSC_CLKBIASDIS_MASK
#endif
            ;

    /* Enable Stop Modes in the Core */
    S32_SCB->SCR |= S32_SCB_SCR_SLEEPDEEP_MASK;//yxl 

    /*  Select VLPS Mode */
    SMC->PMCTRL = SMC_PMCTRL_STOPM(0b10);
    /*
     *
     *  Transition from RUN to VLPR
     *
     *                              */
    if(eRun == SMC->PMSTAT)
    {
        __asm("DSB");
        __asm("ISB");
        /* Call WFI to enter DeepSleep mode */
        __asm("WFI");
    }
    else
    {
//        error_trap();
			// If aborted, reset the MCU.
				S32_SCB->AIRCR = 0x05FA0004;
    }

    /* Verify VLPSA bit is not set */
    if (0 != (SMC->PMCTRL & SMC_PMCTRL_VLPSA_MASK))
    {
//        error_trap();
			// If aborted, reset the MCU.
				S32_SCB->AIRCR = 0x05FA0004;
    }
}

void VLPR_to_Normal(void)//yxl
{
	SMC->PMPROT &= ~SMC_PMPROT_AVLP_MASK;
	PMC->REGSC &= ~PMC_REGSC_BIASEN_MASK;
	SMC->PMCTRL = SMC_PMCTRL_RUNM(0b00);
	
	SOSC_init_8MHz();       /* Initialize system oscilator for 8 MHz xtal */
	SPLL_init_160MHz();     /* Initialize SPLL to 160 MHz with 8 MHz SOSC */
	NormalRUNmode_80MHz();  /* Init clocks: 80 MHz sysclk & core, 40 MHz bus, 20 MHz flash */
}

void Run_to_VLPR (void)
{
    /* Disable clock monitors on SCG module */
    disable_clock_monitors();
    /* Adjust SCG settings to meet maximum frequencies values */
    scg_vlpr_configuration();
    /* Allow very low power run mode */
    SMC->PMPROT |= SMC_PMPROT_AVLP_MASK;
    /* Check if current mode is RUN mode */
    if (eRun == SMC->PMSTAT)
    {
        /* This bit enables source and well biasing for the core logic,
         * this is useful to further reduce MCU power consumption */
        PMC->REGSC |= PMC_REGSC_BIASEN_MASK;
        /* Move to VLPR mode */
        SMC->PMCTRL = SMC_PMCTRL_RUNM(0b10);
        /* Wait for transition */
        while (SMC->PMSTAT != eVLPR) {}
    }
    else
    {
        /* Error trap */
//        error_trap();
			// If aborted, reset the MCU.
				S32_SCB->AIRCR = 0x05FA0004;
    }
}

void VLPR_to_VLPS (void)
{
    uint32_t tempPMC_ctrl = SMC->PMCTRL;
    /* Disable FIRC and SPLL and configure VLPS */
    scg_vlps_configuration();
    /* Enable SLEEPDEEP bit in the Core
     * (Allow deep sleep modes) */
    S32_SCB->SCR |= S32_SCB_SCR_SLEEPDEEP_MASK;
    /* Allow very low power run mode */
    SMC->PMPROT |= SMC_PMPROT_AVLP_MASK;
    /* Select VLPS Mode */
    tempPMC_ctrl &= ~SMC_PMCTRL_STOPM_MASK;
    tempPMC_ctrl |= SMC_PMCTRL_STOPM(0b10);
    SMC->PMCTRL = tempPMC_ctrl;
    /* Reduce power consumption */
    PMC->REGSC |= PMC_REGSC_BIASEN_MASK
#if (0 == SCG_ENABLE_SIRC_IN_VLPS)
            | PMC_REGSC_CLKBIASDIS_MASK
#endif
            ;
    /* Check if current mode is VLPR mode */
    if(eVLPR == SMC->PMSTAT)
    {
        __asm("DSB");
        __asm("ISB");
        /* Go to deep sleep mode */
        asm("WFI");
    }
    else
    {
        /* Error trap */
//        error_trap();
			// If aborted, reset the MCU.
				S32_SCB->AIRCR = 0x05FA0004;
    }
    /* Verify VLPSA bit is not set */
    if (0 != (SMC->PMCTRL & SMC_PMCTRL_VLPSA_MASK))
    {
//        error_trap();
			// If aborted, reset the MCU.
				S32_SCB->AIRCR = 0x05FA0004;
    }
}

void RUN_to_STOP (void)
{
	pnet_config();
	/* Enable SLEEPDEEP bit in the Core
	* (Allow deep sleep modes) */
	S32_SCB ->SCR|=S32_SCB_SCR_SLEEPDEEP_MASK;
	/* Select Stop Mode */
	SMC->PMCTRL=SMC_PMCTRL_STOPM(0b00);
	/* Select which STOP mode (Stop1 or Stop2)
	* is desired (Stop1 - 0b01, Stop2 - 0b10) */
	SMC->STOPCTRL=SMC_STOPCTRL_STOPO(0b01);
	/* Check if current mode is RUN mode */
	if(SMC->PMSTAT == 0x01)
	{
	/* Go to deep sleep mode */
	asm("WFI");
  }
}