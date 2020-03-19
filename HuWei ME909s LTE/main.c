#include "S32K142.h" /* include peripheral declarations S32K144 */
#include "clocks_and_modes.h"
#include "gpio.h"
#include "usertype.h"
#include "uart.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include "mem909s.h"
#define APP_START_ADDRESS  0x00004000

/******************************************************************************
*Local variables
******************************************************************************/

#if 1
void WDOG_disable (void)
{
  WDOG->CNT=0xD928C520;     /* Unlock watchdog 		*///WDOG->CNT=0x0000C620; 
  WDOG->TOVAL=0x0000FFFF;   /* Maximum timeout value 	*/
  WDOG->CS = 0x00002100;    /* Disable watchdog 		*///WDOG->CS = 0x00002140 启动
}
#endif
#if 1 
void WDOG_init()
{
	 WDOG->CNT=0xD928C520;
//	WDOG->CNT=0x0000C620; //Unlock watchdog 并使用内部时钟
	WDOG->TOVAL=0x0000FFFF;
	WDOG->CS = 0x00002140;  //看门狗启动
}
#endif
void feed_WDOG()
{
	WDOG->CNT = 0xB480A602;
	GpioSetVal(PORT_D, 4, PORT_HIGH);
}
void delay(volatile int cycles)
{
    /* Delay function - do nothing for a number of cycles */
    while(cycles--);
}

void ivcu_poweron()
{
	GpioInit();
	PORTD->PCR[0]|=PORT_PCR_MUX(1);
	PORTD->PCR[3]|=PORT_PCR_MUX(1);
	PORTE->PCR[2]|=PORT_PCR_MUX(1);
	PORTE->PCR[3]|=PORT_PCR_MUX(1);
	PORTE->PCR[5]|=PORT_PCR_MUX(1);
	GpioSetDirection(PORT_D, 0, PORT_OUT);
	GpioSetDirection(PORT_D, 3, PORT_IN);
	GpioSetDirection(PORT_E, 2, PORT_OUT);
	GpioSetDirection(PORT_E, 3, PORT_OUT);
	GpioSetDirection(PORT_D, 4, PORT_OUT);
	GpioSetDirection(PORT_E, 5, PORT_OUT);
GpioSetVal(PORT_D, 0, PORT_HIGH);
GpioSetVal(PORT_D, 3, PORT_HIGH);
GpioSetVal(PORT_E, 2, PORT_HIGH);
GpioSetVal(PORT_E, 3, PORT_HIGH);
	GpioSetVal(PORT_E, 5, PORT_HIGH);
}
void Uart_poweron()
{
	GpioInit();
	GpioSetDirection(PORT_D, 0, PORT_OUT);
	GpioSetDirection(PORT_D, 1, PORT_OUT);
	GpioSetDirection(PORT_D, 2, PORT_OUT);
	GpioSetDirection(PORT_D, 3, PORT_OUT);
	GpioSetDirection(PORT_D, 4, PORT_OUT);
	GpioSetDirection(PORT_D, 5, PORT_OUT);
	GpioSetDirection(PORT_D, 6, PORT_OUT);

	GpioSetVal(PORT_D, 0, PORT_HIGH);
	GpioSetVal(PORT_D, 1, PORT_HIGH);
	GpioSetVal(PORT_D, 2, PORT_HIGH);
	GpioSetVal(PORT_D, 3, PORT_HIGH);
	GpioSetVal(PORT_D, 4, PORT_HIGH);
	GpioSetVal(PORT_D, 5, PORT_HIGH);
	GpioSetVal(PORT_D, 6, PORT_HIGH);
	delay(80000);
	GpioSetVal(PORT_D, 1, PORT_LOW);
	GpioSetVal(PORT_D, 2, PORT_LOW);
	delay(80000);
	GpioSetVal(PORT_D, 1, PORT_HIGH);
	GpioSetVal(PORT_D, 2, PORT_HIGH);
	
}
int main(void)
{
		WDOG_disable();
		//WDOG_init();
		SOSC_init_8MHz();       /* Initialize system oscilator for 8 MHz xtal */
	  SPLL_init_160MHz();     /* Initialize SPLL to 160 MHz with 8 MHz SOSC */
	  NormalRUNmode_80MHz();  /* Init clocks: 80 MHz sysclk & core, 40 MHz bus, 20 MHz flash */
		#if 1   //开发板电平拉高
	//	Uart_poweron();
		#endif 
		//while(1);
		ivcu_poweron();
	//while(1);
		uartInit(UART0, 9600);
		uartInit(UART1, 115200);
		uartIrqInit(UART1);
		uartIrqInit(UART0);
	  for(;;)
		{
		mem909sReadyGet();
	  }
		
	return 0;
}
