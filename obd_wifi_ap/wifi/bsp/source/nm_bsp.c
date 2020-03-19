/**
 *
 * \file
 *
 * \brief This module contains SAMD21 BSP APIs implementation.
 *
 * Copyright (c) 2018 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
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
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

#include "bsp/include/nm_bsp.h"
#include "common/include/nm_common.h"
#include "device_registers.h"
#include "stdio.h"
#include "stdlib.h"
#include "gpio.h"
#include "usertype.h"
#include "timer.h"
#include "spi.h"
//#include "winc_init.h"

#ifndef IRQ_PORTCATE
#define EN_PORTCATE		PORT_C
#define EN_PORTNUM		16

#define RESET_PORTCATE		PORT_C
#define RESET_PORTNUM		17

#define CE_PORTCATE		PORT_B
#define CE_PORTNUM		5

#define WAKE_PORTCATE		PORT_B
#define WAKE_PORTNUM		0

#define IRQ_PORTCATE PORT_B
#define IRQ_PORTNUM	1
#endif

static tpfNmBspIsr gpfIsr;

void m2mStub_Pin_init(void){
	GpioInit();
	GpioSetDirection(EN_PORTCATE, EN_PORTNUM, PORT_OUT);
	GpioSetDirection(RESET_PORTCATE, RESET_PORTNUM, PORT_OUT);
	//GpioSetDirection(WAKE_PORTCATE, WAKE_PORTNUM, PORT_OUT);
  GpioSetDirection(IRQ_PORTCATE, IRQ_PORTNUM, PORT_IN);	
}

void m2mStub_time_init(void){
	LPIT0_init();
}

static void chip_isr(void)
{
	#if 0
	if (gpfIsr) {
		gpfIsr();
	}
	#endif
}

/*
 *	@fn		nm_bsp_init
 *	@brief	Initialize BSP
 *	@return	0 in case of success and -1 in case of failure
 */
sint8 nm_bsp_init(void)
{
	#if 0
	gpfIsr = NULL;

	/* Make sure a 1ms Systick is configured. */
	if (!(SysTick->CTRL & SysTick_CTRL_ENABLE_Msk && SysTick->CTRL & SysTick_CTRL_TICKINT_Msk)) {
		delay_init(SysTick);
	}
	#else
	gpfIsr = NULL;
	m2mStub_Pin_init();
	m2mStub_time_init();
	/* Perform chip reset. */
	nm_bsp_reset();
	#endif
	return M2M_SUCCESS;
}

/**
 *	@fn		nm_bsp_deinit
 *	@brief	De-iInitialize BSP
 *	@return	0 in case of success and -1 in case of failure
 */
sint8 nm_bsp_deinit(void)
{
	GpioSetVal(EN_PORTCATE, EN_PORTNUM, PORT_LOW);//pio_set_pin_low(CONF_WINC_PIN_CHIP_ENABLE);
	GpioSetVal(RESET_PORTCATE, RESET_PORTNUM, PORT_LOW);
	return M2M_SUCCESS;
}

/**
 *	@fn		nm_bsp_reset
 *	@brief	Reset NMC1500 SoC by setting CHIP_EN and RESET_N signals low,
 *           CHIP_EN high then RESET_N high
 */
void nm_bsp_reset(void)
{
	GpioSetVal(EN_PORTCATE, EN_PORTNUM, PORT_LOW);//pio_set_pin_low(CONF_WINC_PIN_CHIP_ENABLE);
	GpioSetVal(RESET_PORTCATE, RESET_PORTNUM, PORT_LOW);
	nm_bsp_sleep(2);//1
	GpioSetVal(EN_PORTCATE, EN_PORTNUM, PORT_HIGH);
	nm_bsp_sleep(10);//5
	GpioSetVal(RESET_PORTCATE, RESET_PORTNUM, PORT_HIGH);
	nm_bsp_sleep(15);
}

/*
 *	@fn		nm_bsp_sleep
 *	@brief	Sleep in units of mSec
 *	@param[IN]	u32TimeMsec
 *				Time in milliseconds
 */
void nm_bsp_sleep(uint32 u32TimeMsec)
{
	int ms = 8000 * u32TimeMsec;
	while (ms--){
	}
}

void m2mStub_EintEnable(void){
	PORTB->PCR[1] = 0x000a0100; 
	S32_NVIC->ICPR[(PORTB_IRQn) >> 5] = 1 << ((PORTB_IRQn ) % 32);  /* clr any pending IRQ*/
	S32_NVIC->ISER[(PORTB_IRQn) >> 5] = 1 << ((PORTB_IRQn) % 32);  /* enable IRQ */
	S32_NVIC->IP[PORTB_IRQn] = 0xA;              /* priority 10 of 0-15*/
}
void m2mStub_EintDisable(void){
		S32_NVIC->ICER[(PORTB_IRQn) >> 5] = 1 << ((PORTB_IRQn) % 32);  /* disable IRQ */
}
/*******************************************************************************
  Function:
    void m2mStub_SpiTxRx(uint8_t *p_txBuf, uint16_t txLen, uint8_t *p_rxBuf, uint16_t rxLen)

  Summary:
    Writes and reads bytes from the WINC1500 via the SPI interface

  Description:
    If txLen > rxLen then:
        Throw away the extra read bytes.  Do NOT write the garbage read bytes to p_rxBuf

    If rxLen is > txLen then:
        Write out filler bytes of 0x00 in order to get all the read bytes

  Parameters:
    p_txBuf -- Pointer to tx data (data being clocked out to the WINC1500).
               This will be NULL if txLen is 0.
    txLen   -- Number of Tx bytes to clock out.  This will be 0 if only a read is
               occurring.
    p_rxBuf -- Pointer to rx data (data being clocked in from the WINC1500).
               This will be NULL if rxLen is 0.
    rxLen   -- Number of bytes to read.  This will be 0 if only a write is occurring.

  Returns:
    None
 *****************************************************************************/
void m2mStub_SpiTxRx(uint8_t *p_txBuf, uint16_t txLen, uint8_t *p_rxBuf, uint16_t rxLen){
	if(txLen > 0){
		SpiSend(p_txBuf, txLen, 10);
	}
	if(rxLen > 0){
		SpiRecv(p_rxBuf, rxLen, 10);
	}
}

void PORTB_IRQHandler(void) 
{
	if((PTB->PDIR & (1<<1))==0x00)
	{
		//m2m_EintHandler();
	}
	PORTB->ISFR |= 0x4000;
}

/**
 * \internal Get the PIO hardware instance
 *
 * \param[in] pin The PIO pin
 *
 * \return The instance of PIO hardware
 */

/*
 *	@fn		nm_bsp_register_isr
 *	@brief	Register interrupt service routine
 *	@param[IN]	pfIsr
 *				Pointer to ISR handler
 */
void nm_bsp_register_isr(tpfNmBspIsr pfIsr)
{
	#if 0
	gpfIsr = pfIsr;

	ext_irq_register(CONF_WINC_EXT_INT_PIN, chip_isr);
	#endif
}

/*
 *	@fn		nm_bsp_interrupt_ctrl
 *	@brief	Enable/Disable interrupts
 *	@param[IN]	u8Enable
 *				'0' disable interrupts. '1' enable interrupts
 */
void nm_bsp_interrupt_ctrl(uint8 u8Enable)
{
	if(u8Enable == 1){
		m2mStub_EintEnable();
	}
	else if(u8Enable == 0){
		m2mStub_EintDisable();
	}
}
