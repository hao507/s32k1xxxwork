#include "stdlib.h"
#include "gpio.h"
#include "usertype.h"
#include "device_registers.h"
#include "winc1500_api.h"

#define IRQ_PORT_CAT PORT_B
#define IRQ_PORT_NUM	1

void gpioIrq_Init(void) 
{ 	
	PCC->PCCn[PCC_PORTB_INDEX] |= PCC_PCCn_CGC_MASK;
	PTB->PDDR &= ~(1<<1);  
  GpioSetDirection(IRQ_PORT_CAT, IRQ_PORT_NUM, PORT_IN);	
	PORTB->PCR[1] = 0x000a0100; 
	
	S32_NVIC->ICPR[(PORTB_IRQn) >> 5] = 1 << ((PORTB_IRQn ) % 32);  /* clr any pending IRQ*/
	S32_NVIC->ISER[(PORTB_IRQn) >> 5] = 1 << ((PORTB_IRQn) % 32);  /* enable IRQ */
	S32_NVIC->IP[PORTB_IRQn] = 0xA;              /* priority 10 of 0-15*/
}

void PORTB_IRQHandler(void) 
{
	if((PTB->PDIR & (1<<1))==0x00)
	{
		m2m_EintHandler();
	}
	PORTB->ISFR |= 0x4000;
}
