/*
 * uart.c
 *
 *  Created on: 2019Äê5ÔÂ14ÈÕ
 *      Author: yxl
 */
#include "timer.h"
#include "device_registers.h" /* include peripheral declarations S32K144 */
#include "usertype.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define TIMER_MAX	10
#define TIMER_OUT	1
static int timerNum = 0, timerMax = 0, heartbeat = 0;
static int timer[TIMER_MAX] = {0}, timerFlag[TIMER_MAX] = {0};
static int timeInitFlag = 0;

void LPTMR_init(void)//2ms
{
	// 64
    PCC->PCCn[PCC_LPTMR0_INDEX] = PCC_PCCn_CGC_MASK;	/* Enable LPTMR Clock 		*/
    LPTMR0->PSR |= LPTMR_PSR_PCS(1)                  /* LPTMR clk src: 1KHz LPO  0b01*/
                  |LPTMR_PSR_PBYP_MASK;               	/* Bypass Prescaler 		*/
    LPTMR0->CMR = 1;//time_ms - 1;                                  /* 500 ms interrupt 		*/
    LPTMR0->CSR |= LPTMR_CSR_TIE_MASK; 					/* Timer interrupt enabled */
    LPTMR0->CSR |= LPTMR_CSR_TEN_MASK;                  /* Enable Timer 			*/
		 LPTMR0->CSR |= LPTMR_CSR_TCF(0)| LPTMR_CSR_TPP(0)| LPTMR_CSR_TFC(1);
		S32_NVIC->ISER[(uint32_t)(LPTMR0_IRQn) >> 5U] = (uint32_t)(1U << ((uint32_t)(LPTMR0_IRQn) & (uint32_t)0x1FU));
		S32_NVIC->ICPR[(uint32_t)(LPTMR0_IRQn) >> 5U] = (uint32_t)(1U << ((uint32_t)(LPTMR0_IRQn) & (uint32_t)0x1FU));
		timeInitFlag = 1;
}

void LPTMR_Reset()
{

		//PCC->PCCn[PCC_LPTMR0_INDEX] = ~PCC_PCCn_CGC_MASK;	/* Enable LPTMR Clock 		*/ 
//    LPTMR0->PSR |= ~LPTMR_PSR_PCS(1)                  /* LPTMR clk src: 1KHz LPO  0b01*/
//                  |~LPTMR_PSR_PBYP_MASK;               	/* Bypass Prescaler 		*/	/* 500 ms interrupt 		*/
//    LPTMR0->CSR |= ~LPTMR_CSR_TIE_MASK; 					/* Timer interrupt enabled */
//    LPTMR0->CSR |= ~LPTMR_CSR_TEN_MASK;                  /* Enable Timer 			*/
		timeInitFlag = 0;               /* disable Timer 			*/
			S32_NVIC->ICER[(uint32_t)(LPTMR0_IRQn) >> 5U] = (uint32_t)(1U << ((uint32_t)(LPTMR0_IRQn) & (uint32_t)0x1FU));
}

int  TimerCreate(unsigned int time_2ms){
	int i = 0, temp = 0;
	if(timeInitFlag != 1)
		return -3;
	for(i=0; i < timerNum; i++){
		if(timer[i] == time_2ms)
			return -2;
	}
	if(timerNum < TIMER_MAX)
		timer[timerNum++] = time_2ms;
	else
		return -1;
	for(i=0; i< timerNum; i++){
		if(i>0){
			if(timer[i] > timer[i-1] && timer[i] > temp){
				temp = timer[i];
				timerMax = i;
			}
		}
	}
	return timerNum-1;
}

int  TimerDelete(unsigned int time_2ms){
	int i = 0;
	for(i=0; i< timerNum; i++){
		if(timer[i] == time_2ms){
			memcpy(timer+i, timer+i+1, sizeof(int)*(timerNum - i - 1));
			memcpy(timerFlag+i, timerFlag+i+1, sizeof(int)*(timerNum - i - 1));
			timerNum = timerNum - 1;
			return 0;
		}
	}
	return -1;
}
int TimerOutGet(unsigned int time_2ms)
{
	int ret = -1, i = 0;
	for(i=0; i < timerNum; i++){
		if(timer[i] == time_2ms){
			ret = timerFlag[i];
			timerFlag[i] = 0;
			return ret;
		}
	}
	return ret;
}
int modeTemp = 0;
void LPTMR0_IRQHandler (void)
{
	if(0!=(LPTMR0->CSR & LPTMR_CSR_TCF_MASK))
	{
		modeTemp = SMC->PMSTAT;
		/* Check if TCF flag is set */
		LPTMR0->CSR |= LPTMR_CSR_TCF_MASK;	/*	Clear TCF flag by writting a logic one */
		heartbeat++;
		int i = 0;
		for(i=0; i< timerNum; i++){
			if(heartbeat == timer[i]){
				timeOutNum[timerNum]++;
			}
		}
		if(heartbeat == timer[timerMax])
			heartbeat = 0;
	}
}

//-------------------------------------
static int timerLpitNum = 0, timerLpitMax = 0, heartbeatLpit = 0;
static int timerLpit[TIMER_MAX] = {0}, timerLpitFlag[TIMER_MAX] = {0};
static int timeLpitInitFlag = 0;
//--------------------------------------
//if want to change the timer value, change TVAL
void LPIT0_init (void) // 1ms
{
	PCC->PCCn[PCC_LPIT_INDEX] = PCC_PCCn_PCS(6);    // LPIT0?? Clock Src = 6 (SPLL2_DIV2_CLK=160MHZ/4 = 40MHZ)
	PCC->PCCn[PCC_LPIT_INDEX] |= PCC_PCCn_CGC_MASK; //??LPIT0??
	LPIT0->MCR = 0x00000001;    // DBG_EN-0: ??????????????
                                // DOZE_EN=0: ?????????????? 
                                // SW_RST=0: ????????????
                                // M_CEN=1: ?????????
	LPIT0->MIER = 0x00000001;   // TIE0=1: ???LPIT ??0????
	LPIT0->TMR[0].TVAL = 40000;     // ???????: 4M clocks
	LPIT0->TMR[0].TCTRL = 0x00000001; // T_EN=1: ?????
	
	S32_NVIC->ISER[(uint32_t)(LPIT0_Ch0_IRQn) >> 5U] = (uint32_t)(1U << ((uint32_t)(LPIT0_Ch0_IRQn) & (uint32_t)0x1FU));
  S32_NVIC->ISER[(uint32_t)(LPIT0_Ch0_IRQn) >> 5U] = (uint32_t)(1UL << ((uint32_t)(LPIT0_Ch0_IRQn) & (uint32_t)0x1FU));
  S32_NVIC->IP[LPIT0_Ch0_IRQn] = 0x8;              /* IRQ48-LPIT0 ch0: priority 8 of 0-15*/
	timeInitFlag = 1;
}

int  TimerLpitCreate(unsigned int time_ms){	
	int i = 0, temp = 0;
	if(timeLpitInitFlag != 1)
		return -3;
	for(i=0; i < timerLpitNum; i++){
		if(timerLpit[i] == time_ms)
			return -2;
	}
	if(timerLpitNum < TIMER_MAX)
		timerLpit[timerLpitNum++] = time_ms;
	else
		return -1;
	for(i=0; i< timerLpitNum; i++){
		if(i>0){
			if(timerLpit[i] > timerLpit[i-1] && timerLpit[i] > temp){
				temp = timerLpit[i];
				timerLpitMax = i;
			}
		}
	}
	return timerLpitNum-1;
}

int  TimerLpitDelete(unsigned int time_ms){
	int i = 0;
	for(i=0; i< timerLpitNum; i++){
		if(timerLpit[i] == time_ms){
			memcpy(timerLpit+i, timerLpit+i+1, sizeof(int)*(timerLpitNum - i - 1));
			memcpy(timerLpitFlag+i, timerLpitFlag+i+1, sizeof(int)*(timerLpitNum - i - 1));
			timerLpitNum = timerLpitNum - 1;
			return 0;
		}
	}
	return -1;
}
int TimerLpitOutGet(unsigned int time_ms)
{
	int ret = -1, i = 0;
	for(i=0; i < timerLpitNum; i++){
		if(timerLpit[i] == time_ms){
			ret = timerLpitFlag[i];
			timerLpitFlag[i] = 0;
			return ret;
		}
	}
	return ret;
}

void LPIT0_Ch0_IRQHandler (void)
{
	if(0!=(LPIT0->MSR & LPIT_MSR_TIF0_MASK)){
		LPIT0->MSR |= LPIT_MSR_TIF0_MASK; /* ??????*/
		heartbeatLpit++;
		int i = 0;
		for(i=0; i< timerLpitNum; i++){
			if(heartbeatLpit == timerLpit[i]){
				timeOutNum[timerLpitNum]++;
			}
		}
		if(heartbeatLpit == timerLpit[timerMax])
			heartbeatLpit = 0;
	}
}
uint16_t CurrentCaptureVal = 0;
uint16_t PriorCaptureVal = 0;
uint16_t DeltaCapture = 0;

void FTM0_init(void)
{
	/**
	 * FTM0 Clocking:
	 * ==================================================
	 */
	PCC->PCCn[PCC_FTM0_INDEX] &= ~PCC_PCCn_CGC_MASK; 	/* Ensure clk disabled for config 	*/
	PCC->PCCn[PCC_FTM0_INDEX] |= PCC_PCCn_PCS(0b010)	/* Clock Src=1, 8 MHz SIRCDIV1_CLK */
                              |  PCC_PCCn_CGC_MASK;  	/* Enable clock for FTM regs 		*/

	/*!
	 * FTM0 Initialization:
	 * ===================================================
	 */
	FTM0->MODE |= FTM_MODE_WPDIS_MASK;  /* Write protect to registers disabled (default) 				*/
	FTM0->SC	=	FTM_SC_PWMEN0_MASK	/* Enable PWM channel 0 output									*/
					|FTM_SC_PWMEN1_MASK /* Enable PWM channel 1 output									*/
					|FTM_SC_PS(3)
					|FTM_SC_TOIE(1);     	/* TOIE (Timer Overflow Interrupt Ena) = 0 (default) 			*/
										/* CPWMS (Center aligned PWM Select) = 0 (default, up count) 	*/
										/* CLKS (Clock source) = 0 (default, no clock; FTM disabled) 	*/
										/* PS (Prescaler factor) = 7. Prescaler = 128 					*/

	FTM0->COMBINE = 0x00000000;	/* FTM mode settings used: DECAPENx, MCOMBINEx, COMBINEx=0 */
	FTM0->POL = 0x00000000;    	/* Polarity for all channels is active high (default) 		*/
	FTM0->MOD = 100 -1 ;     	/* FTM1 counter final value (used for PWM mode) 			*///10ms
								/* FTM1 Period = MOD-CNTIN+0x0001 ~= 62500 ctr clks  		*/
								/* 8MHz /128 = 62.5kHz ->  ticks -> 1Hz 					*/
								
	S32_NVIC->ISER[(uint32_t)(FTM0_Ovf_Reload_IRQn) >> 5U] = (uint32_t)(1U << ((uint32_t)(FTM0_Ovf_Reload_IRQn) & (uint32_t)0x1FU));
	S32_NVIC->ICPR[(uint32_t)(FTM0_Ovf_Reload_IRQn) >> 5U] = (uint32_t)(1U << ((uint32_t)(FTM0_Ovf_Reload_IRQn) & (uint32_t)0x1FU));
}

void FTM0_CH0_OC_init(void)
{
	/**
	 * FTM0, Channel 0 in Output Compare Mode:
	 * =================================================
	 */
	FTM0->CONTROLS[0].CnSC = FTM_CnSC_MSA_MASK
							|FTM_CnSC_ELSA_MASK;/* FTM0 ch0: Output Compare, toggle output on match 	*/
												/* CHIE (Chan Interrupt Ena)= 0 (default) 				*/
												/* MSB:MSA (chan Mode Select)= 0b01, Output Compare 	*/
												/* ELSB:ELSA (chan Edge or Level Select)= 0b01, toggle	*/

	FTM0->CONTROLS[0].CnV= 10;         /* FTM0 ch 0 Compare Value= 6250  clks, 100ms toggle	*/
	FTM0->POL &= ~FTM_POL_POL0_MASK;     /* FTM0 ch 0 polarity = 0 (Default, active high) 		*/
}

void FTM0_CH1_PWM_init(void)
{
	/**
	 * FTM0, Channel 1 in PWM Mode:
	 * ==================================================
	 */
	FTM0->CONTROLS[1].CnSC = FTM_CnSC_MSB_MASK
							|FTM_CnSC_ELSB_MASK;  	/* FTM0 ch1: edge-aligned PWM, low true pulses 		*/
													/* CHIE (Chan Interrupt Ena) = 0 (default) 			*/
													/* MSB:MSA (chan Mode Select)=0b10, Edge Align PWM		*/
													/* ELSB:ELSA (chan Edge/Level Select)=0b10, low true 	*/

	FTM0->CONTROLS[1].CnV =  46875;	/* FTM0 ch1 compare value (~75% duty cycle) */
}

void FTM0_CH6_IC_init(void)
{
	/**
	 * FTM0, Channel 6 in Input Capture Mode:
	 * =================================================
	 */
	FTM0->CONTROLS[6].CnSC = FTM_CnSC_ELSB_MASK
							|FTM_CnSC_ELSA_MASK;  	/* FTM0 ch6: Input Capture rising or falling edge 		*/
													/* CHIE (Chan Interrupt Ena) = 0 (default) 			*/
													/* MSB:MSA (chan Mode Select)=0b00, Input Capture 		*/
													/* ELSB:ELSA (ch Edge/Level Select)=0b11, rise or fall	*/
}

#if 0
void FTM0_CH0_output_compare(void)
{
	if (1==((FTM0->CONTROLS[0].CnSC & FTM_CnSC_CHF_MASK)>>FTM_CnSC_CHF_SHIFT))
	{
														/* - If chan flag is set: 				*/
		FTM0->CONTROLS[0].CnSC &= ~FTM_CnSC_CHF_MASK;  	/* Clear flag: read reg then set CHF=0 */
		if(  FTM0->CONTROLS[0].CnV==56250)
		{
										/* - If count at last value before end: 	*/
			FTM0->CONTROLS[0].CnV= 0 ;  /* Update compare value: to 0				*/
		}
		else
		{
			FTM0->CONTROLS[0].CnV= FTM0->CONTROLS[0].CnV + 6250 ;
			/* - else: */
			/* Update compare value: add 6250 to current value	*/
		}
	}
}
#else
static int timeoutNum = 0;
int temp = 0;
void FTM0_CH0_output_compare(void)
{
	if (1==((FTM0->CONTROLS[0].CnSC & FTM_CnSC_CHF_MASK)>>FTM_CnSC_CHF_SHIFT)){
		FTM0->CONTROLS[0].CnSC &= ~FTM_CnSC_CHF_MASK;  	/* Clear flag: read reg then set CHF=0 */
		temp = FTM0->CONTROLS[0].CnV;
		timeoutNum++;
		if(  FTM0->CONTROLS[0].CnV==10000-100)
		{
										/* - If count at last value before end: 	*/
			FTM0->CONTROLS[0].CnV= 0 ;  /* Update compare value: to 0				*/
		}
		else
		{
			FTM0->CONTROLS[0].CnV= FTM0->CONTROLS[0].CnV + 100 ;
			/* - else: */
			/* Update compare value: add 6250 to current value	*/
		}
		if(timeoutNum == 1000){
			GpioSetVal(PORT_D, 5, PORT_LOW);
		}
		else if(timeoutNum == 2000){
			GpioSetVal(PORT_D, 5, PORT_HIGH);
		}
		else if(timeoutNum == 10000){
			timeoutNum = 0;
		}
	}
}

void FTM0_Ovf_Reload_IRQHandler(void){
	if(FTM0->SC & FTM_SC_TOF_MASK){
		FTM0->SC &= (~FTM_SC_TOF_MASK);
		temp = FTM0->CONTROLS[0].CnV;
//		FTM0_CH0_output_compare();
		timeoutNum++;
		if (1==((FTM0->CONTROLS[0].CnSC & FTM_CnSC_CHF_MASK)>>FTM_CnSC_CHF_SHIFT)){
			FTM0->CONTROLS[0].CnSC &= ~FTM_CnSC_CHF_MASK; 
		}
		if(  FTM0->CONTROLS[0].CnV==100-10)
		{
										/* - If count at last value before end: 	*/
			FTM0->CONTROLS[0].CnV= 0 ;  /* Update compare value: to 0				*/
		}
		else
		{
			FTM0->CONTROLS[0].CnV= FTM0->CONTROLS[0].CnV + 100 ;
			/* - else: */
			/* Update compare value: add 6250 to current value	*/
		}
		if(timeoutNum == 1000){
			GpioSetVal(PORT_D, 5, PORT_HIGH);
		}
		else if(timeoutNum == 2000){
			GpioSetVal(PORT_D, 5, PORT_LOW);
		}
		else if(timeoutNum == 9923){
			timeoutNum = 0;
		}
	}
}
#endif

void FTM0_CH6_input_capture(void)
{
	if (1==((FTM0->CONTROLS[6].CnSC & FTM_CnSC_CHF_MASK)>>FTM_CnSC_CHF_SHIFT))
	{
														/* - If chan flag is set 				*/
		FTM0->CONTROLS[6].CnSC &= ~FTM_CnSC_CHF_MASK;   /* Clear flag: read reg then set CHF=0 */
		PriorCaptureVal = CurrentCaptureVal;            /* Record value of prior capture 		*/
		CurrentCaptureVal = FTM0->CONTROLS[6].CnV;      /* Record value of current capture 	*/

		DeltaCapture = CurrentCaptureVal - PriorCaptureVal;
		/* Will be 6250 clocks (100 msec) if connected to FTM0 CH0 */
	}
}

void start_FTM0_counter (void)
{
	FTM0->SC |= FTM_SC_CLKS(3);
	/* Start FTM0 counter with clk source = external clock (SOSCDIV1_CLK)*/
}