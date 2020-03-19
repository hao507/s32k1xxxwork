/*
 * uart.c
 *
 *  Created on: 2019Äê8ÔÂ2ÈÕ
 *      Author: yxl
 */
#include "device_registers.h" /* include peripheral declarations S32K144 */
#include "usertype.h"
#include "s32_core_cm4.h"
#include "watchdog.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


static inline int wdog_IsUnlocked(void)
{
	return ((WDOG->CS & WDOG_CS_ULK_MASK) >> WDOG_CS_ULK_SHIFT) != 0U;
}
static inline void wdog_SetWindowMode(bool enable)
{
    WDOG->CS |= WDOG_CS_WIN(enable ? 1UL : 0UL);
}

static inline void wdog_SetWindowValue(int value)
{
		WDOG->WIN = value;
}

static inline bool wodg_IsUpdateEnabled(void)
{
    return ((WDOG->CS & WDOG_CS_UPDATE_MASK) >> WDOG_CS_UPDATE_SHIFT) != 0U;
}

/*
DisableInterrupts; // disable global interrupt
WDOG_CNT = 0xD928C520; //unlock watchdog
while(WDOG_CS[ULK]==0); //wait until registers are unlocked
WDOG_TOVAL = 256; //set timeout value
WDOG_CS = WDOG_CS_EN(1) | WDOG_CS_CLK(1) | WDOG_CS_INT(1) |
WDOG_CS_WIN(0) | WDOG_CS_UPDATE(0);
while(WDOG_CS[RCS]==0); //wait until new configuration takes effect
EnableInterrupts; //enable global interrupt
*/
 #if 0
 __asm void INTERRUPTS_ENABLE(void){ 
	 CPSIE I;
 }
 __asm void INTERRUPTS_DISABLE(void){ 
	 CPSID I;
 }
int wdog_enable(void)
{
	INTERRUPTS_DISABLE();
	WDOG->CNT = FEATURE_WDOG_UNLOCK_VALUE;
	while(!(WDOG_CS_ULK(1)));
	WDOG->TOVAL = 256;
	WDOG->CS = WDOG_CS_EN(1) | WDOG_CS_CLK(1) | WDOG_CS_INT(1) |
  WDOG_CS_WIN(0) | WDOG_CS_UPDATE(0);
  while(WDOG_CS_RCS(1)==0);
	INTERRUPTS_ENABLE();
}
#else
/*
	the win bit is write-once bit enables window mode.if we set the win mode, we must set 1 at first, if set 0 ,then set 1 will be failed.
	we can set wdogUserConfig like:
		const wdog_user_config_t wdogUserConfig = {
    .clkSource = WDOG_LPO_CLOCK,
    .opMode = {
        false, // Wait Mode
        false, // Stop Mode
        false // Debug Mode
    },
    .updateEnable = true,
    .intEnable = true,
    .winEnable = false,
    .windowValue = 20000,
    .timeoutValue = 60000,
    .prescalerEnable = false
};
*/
int wdog_enable(wdog_user_config_t wdogUserConfig)
{
		int ret = -1;
    uint32_t cs = WDOG->CS;
	

    if ((wodg_IsUpdateEnabled()) && (wdogUserConfig.timeoutValue > FEATURE_WDOG_MINIMUM_TIMEOUT_VALUE))
    {
        /* Clear the bits used for configuration */
        cs &= ~(WDOG_CS_WIN_MASK | WDOG_CS_PRES_MASK | WDOG_CS_CLK_MASK | WDOG_CS_INT_MASK |
                WDOG_CS_UPDATE_MASK | WDOG_CS_DBG_MASK | WDOG_CS_WAIT_MASK | WDOG_CS_STOP_MASK);
        /* Construct CS register new value */
        cs |= WDOG_CS_WIN(wdogUserConfig.winEnable ? 1UL : 0UL);
        cs |= WDOG_CS_PRES(wdogUserConfig.prescalerEnable ? 1UL : 0UL);
        cs |= WDOG_CS_CLK(wdogUserConfig.clkSource);
        cs |= WDOG_CS_INT(wdogUserConfig.intEnable ? 1UL : 0UL);
        cs |= WDOG_CS_UPDATE(wdogUserConfig.updateEnable ? 1UL : 0UL);
        if (wdogUserConfig.opMode.debug)
        {
            cs |= WDOG_CS_DBG_MASK;
        }
        if (wdogUserConfig.opMode.wait)
        {
            cs |= WDOG_CS_WAIT_MASK;
        }
        if (wdogUserConfig.opMode.stop)
        {
            cs |= WDOG_CS_STOP_MASK;
        }
        /* Reset interrupt flags */
        cs |= WDOG_CS_FLG_MASK;
        /* Enable WDOG in 32-bit mode */
        cs |= WDOG_CS_EN_MASK | WDOG_CS_CMD32EN_MASK;

        wdog_unlock();

        while (!wdog_IsUnlocked())
        {
          /* Wait until registers are unlocked */
        }
        WDOG->CS = cs;
				
				//wdog_SetWindow(true, 20000);
        WDOG->TOVAL = wdogUserConfig.timeoutValue;

        if (wdogUserConfig.winEnable)
        {
            WDOG->WIN = wdogUserConfig.windowValue;
        }

        while (!wdog_IsUnlocked())
        {
          /* Wait until the unlock window closes */
        }
    }
    return ret;
}

#endif


void wdog_unlock(void)
{
	if (((WDOG)->CS & WDOG_CS_CMD32EN_MASK) != 0U) 
    { 
        WDOG->CNT = FEATURE_WDOG_UNLOCK_VALUE;
				(void)WDOG->CNT;			
    } 
    else 
    { 
        WDOG->CNT = FEATURE_WDOG_UNLOCK16_FIRST_VALUE;
				(void)WDOG->CNT; 
				WDOG->CNT = FEATURE_WDOG_UNLOCK16_SECOND_VALUE; 
				(void)WDOG->CNT; 
    } 
}
void wdog_disable(void)
{
	WDOG->CNT=0xD928C520;     /* Unlock watchdog 		*/
  WDOG->TOVAL=0x0000FFFF;   /* Maximum timeout value 	*/
  WDOG->CS = 0x00002100;    /* Disable watchdog 		*/
}

void wdog_refresh(void)
{
	if ((WDOG->CS & WDOG_CS_CMD32EN_MASK) != 0U)
    {
        WDOG->CNT = FEATURE_WDOG_TRIGGER_VALUE;
    }
    else
    {
        WDOG->CNT = FEATURE_WDOG_TRIGGER16_FIRST_VALUE;
        //(void)WDOG->CNT;
        WDOG->CNT = FEATURE_WDOG_TRIGGER16_SECOND_VALUE;
    }
}

uint16_t wodg_GetCounter(void)
{
    return (uint16_t)WDOG->CNT;
}

void wdog_SetTimeout(uint16_t timeout)
{
    wdog_unlock();
    WDOG->TOVAL = timeout;
}

void wdog_SetWindow(bool enable, uint16_t windowvalue)
{
    /* Set WDOG window mode */
    wdog_SetWindowMode(enable);

    /* If enable window mode */
    if(enable)
    {
        /* Set window value for the WDOG */
        wdog_SetWindowValue(windowvalue);
    }
}

//--timeout interput----
int wodg_SetIntrupt(bool enable)
{
    int status = 0;
    if (wodg_IsUpdateEnabled())
    {
        /* Enable/disable WDOG interrupt */
				WDOG_CS_INT(enable);
        while (wdog_IsUnlocked())
        {
            /* Wait until the unlock window closes */
        }
    }
    else
    {
        status = -1;
    }

    return status;
}


#if 1
void EWM_init(void)
{
  PCC->PCCn[PCC_PORTE_INDEX] = PCC_PCCn_CGC_MASK; 	/* Enable clock for PORT A 		*/
  PORTE->PCR[4] |= PORT_PCR_MUX(7);  				/* Port A2: MUX = ALT4, EWM_OUT_b  */
  	  	  	  	  	  	  	  	  	  	  	  	  	/* External Watchdog Monitor Output */
	/*!
	 *EWM clocking
	 *==========================================
	 */
	PCC->PCCn[PCC_EWM_INDEX] |= PCC_PCCn_CGC_MASK;
	EWM->CLKPRESCALER = 127;	/*! Prescaled clock frequency =
								 *	low power clock source frequency / ( 1 + CLK_DIV ) =
								 *  LPO 128KHz / 128 = 1KHz.
								 */

	/*!
	 * The EWM compare registers are used to create a
	 * refresh window to refresh the EWM module.
	 */
	EWM->CMPH = 0xFE; 	/* When the counter passes this value outb will be asserted */
	EWM->CMPL = 0x0F;	/* Refresh window is [15ms - 255ms]. */

	EWM->CTRL |= EWM_CTRL_INTEN_MASK| 	/* Enable interrupt */
				 EWM_CTRL_EWMEN_MASK; 	/* Enable EWM */
}

void WDOG_EWM_IRQHandler(void)
{
	/* EWM_OUT_b is asserted */
	EWM->CTRL &= ~EWM_CTRL_INTEN_MASK;	/* de-assert interrupt request */
										/* does not affect EWM_OUT_b. */
	
}

void WDOG_EWM_Enable_Interrupt(void)
{
	S32_NVIC->ISER[(uint32_t)(WDOG_EWM_IRQn) >> 5U] = (uint32_t)(1U << ((uint32_t)(WDOG_EWM_IRQn) & (uint32_t)0x1FU));
	S32_NVIC->ICPR[(uint32_t)(WDOG_EWM_IRQn) >> 5U] = (uint32_t)(1U << ((uint32_t)(WDOG_EWM_IRQn) & (uint32_t)0x1FU));
}
#endif
