#include "S32K142.h" /* include peripheral declarations S32K144 */
#include "clocks_and_modes.h"
#include "gpio.h"
#include "usertype.h"
#include "uart.h"
#include "updata.h"
#include <string.h>
#include "mem909s.h"
#include "Type0_mcu_Test.h"
#include "Type1_mcu_Test.h"
#include "wdgTps.h"
#include "canbus.h"
#include "timer.h"
#include "updata.h"
#include "mem909s.h"
#include "mcu_power_massge.h"
#include "type1_power.h"
#define UPDATA_START_ADDRESS  0x00000000

/******************************************************************************
*Local variables
******************************************************************************/
typedef void (*bootloader_fun)(void);
bootloader_fun jump2app;

void WDOG_disable (void)
{
  WDOG->CNT=0xD928C520;     /* Unlock watchdog 		*/
  WDOG->TOVAL=0x0000FFFF;   /* Maximum timeout value 	*/
  WDOG->CS = 0x00002100;    /* Disable watchdog 		*/
}

void delay(volatile int cycles)
{
    /* Delay function - do nothing for a number of cycles */
    while(cycles--);
}

void disable_irq()
{
//	LPTMR_Reset();
	Canbus0_irq_Reset();
	Canbus1_irq_Reset();
	UART_Reset(UART0);
	UART_Reset(UART1);
}
void ivcu_poweron()
{
	GpioInit();
	GpioSetDirection(PORT_D, 0, PORT_OUT);
	GpioSetDirection(PORT_D, 1, PORT_OUT);
	GpioSetDirection(PORT_D, 2, PORT_OUT);
	GpioSetDirection(PORT_D, 3, PORT_OUT);
	GpioSetDirection(PORT_D, 4, PORT_OUT);
	GpioSetDirection(PORT_D, 5, PORT_OUT);
	GpioSetDirection(PORT_D, 6, PORT_OUT);
	GpioSetDirection(PORT_D, 16, PORT_OUT);
	GpioSetDirection(PORT_E,8,PORT_OUT);			//MCU Dormancy control
	GpioSetDirection(PORT_D,7,PORT_OUT);
	GpioSetVal(PORT_D, 0, PORT_HIGH);
	GpioSetVal(PORT_D, 1, PORT_HIGH);
	GpioSetVal(PORT_D, 2, PORT_HIGH);
	GpioSetVal(PORT_D, 3, PORT_HIGH);
	GpioSetVal(PORT_D, 4, PORT_HIGH);
	GpioSetVal(PORT_D, 5, PORT_HIGH);
	GpioSetVal(PORT_D, 6, PORT_HIGH);
	GpioSetVal(PORT_D,7, PORT_HIGH);
	GpioSetVal(PORT_E,8, PORT_LOW);
	delay(80000);
	GpioSetVal(PORT_D, 1, PORT_LOW);
	GpioSetVal(PORT_D, 2, PORT_LOW);
	delay(80000);
	GpioSetVal(PORT_D, 1, PORT_HIGH);
	GpioSetVal(PORT_D, 2, PORT_HIGH);
}

int main(void)
{
		SOSC_init_8MHz();       /* Initialize system oscilator for 8 MHz xtal */
	  SPLL_init_160MHz();     /* Initialize SPLL to 160 MHz with 8 MHz SOSC */
	  NormalRUNmode_80MHz();  /* Init clocks: 80 MHz sysclk & core, 40 MHz bus, 20 MHz flash */
		LPTMR_init();						/*WODG time_init*/
		ivcu_poweron();
		wdgTps_init();
		uartInit(UART1,9600);
		uartIrqInit(UART1);
		Canbus0Init(500000);
		Canbus0IrqInit();
		type1_powerinit();
			for(;;)
		{
			wdgTps_process();
			type1mcu_receive_type0mcu();
//			if(send_zynq_type1_flag == 1){
//				send_zynq_request_type1();
//			}
			receive_zynq_type1();
		}
	return 0;
}

