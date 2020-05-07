/*
 * Copyright (c) 2014 - 2016, Freescale Semiconductor, Inc.
 * Copyright (c) 2016 - 2018, NXP.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY NXP "AS IS" AND ANY EXPRESSED OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL NXP OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "device_registers.h"	/* include peripheral declarations */
#include "FTM.h"
#include "gpio.h"
#include "usertype.h"
#include <stdio.h>
#include "LPUART_S32K11X.h"
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
unsigned long utc = 0;
float utcVal = 0.0;
char sendBuf[120];
unsigned char crcVal = 0;
char crc[8];
unsigned char gps_crc(unsigned char *pdata, int len)
{
    unsigned char crc = 0;
    int i=0;
    if(pdata == NULL && len <= 0)   return -1;

    for(i=0; i<len; i++){
        crc ^= pdata[i];
    }
    return crc;
}
void wdgTps_process()
{
	if(timeoutNum == 4000){
						utc++;
            utcVal = utc/(60*60) * 10000 + (utc % (60*60))/60 * 100+ utc%(60);
            if(utcVal < 10)
                sprintf(sendBuf, "$GPRMC,00000%.2f,A,3158.4608,N,11848.3737,E,10.05,324.27,150706,,,A*",utcVal);
            else if(utcVal < 100)
                sprintf(sendBuf, "$GPRMC,0000%.2f,A,3158.4608,N,11848.3737,E,10.05,324.27,150706,,,A*",utcVal);
            else if(utcVal < 1000)
                sprintf(sendBuf, "$GPRMC,000%.2f,A,3158.4608,N,11848.3737,E,10.05,324.27,150706,,,A*",utcVal);
            else if(utcVal < 10000)
                sprintf(sendBuf, "$GPRMC,00%.2f,A,3158.4608,N,11848.3737,E,10.05,324.27,150706,,,A*",utcVal);
            else if(utcVal < 100000)
                sprintf(sendBuf, "$GPRMC,0%.2f,A,3158.4608,N,11848.3737,E,10.05,324.27,150706,,,A*",utcVal);
            else
                sprintf(sendBuf, "$GPRMC,%.2f,A,3158.4608,N,11848.3737,E,10.05,324.27,150706,,,A*",utcVal);
						crcVal = gps_crc((unsigned char *)sendBuf+1, strlen((unsigned char *)sendBuf)-2);//2
						sprintf(crc,"%02x\n\r",crcVal);
						strcat(sendBuf, crc);
						if(utcVal >= 235959.0)  utc = 0;
						LPUART0_transmit_string(sendBuf);
	}
						
						
}