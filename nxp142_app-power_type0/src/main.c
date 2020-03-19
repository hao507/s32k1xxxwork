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
//	GpioSetDirection(PORT_D,7,PORT_OUT);
	GpioSetVal(PORT_D, 0, PORT_HIGH);
	GpioSetVal(PORT_D, 1, PORT_HIGH);
	GpioSetVal(PORT_D, 2, PORT_HIGH);
	GpioSetVal(PORT_D, 3, PORT_HIGH);
	GpioSetVal(PORT_D, 4, PORT_HIGH);
	GpioSetVal(PORT_D, 5, PORT_HIGH);
	GpioSetVal(PORT_D, 6, PORT_HIGH);
//	GpioSetVal(PORT_D,7, PORT_HIGH);
	GpioSetVal(PORT_E,8, PORT_LOW);
	GpioSetVal(PORT_D, 16,PORT_HIGH);
	delay(80000);
	GpioSetVal(PORT_D, 1, PORT_LOW);
	GpioSetVal(PORT_D, 2, PORT_LOW);
	delay(80000);
	GpioSetVal(PORT_D, 1, PORT_HIGH);
	GpioSetVal(PORT_D, 2, PORT_HIGH);
}


int sendStatus = 0;
char Lte_rData[200]={0};
	int rLenNow1 = 0;
void read ();
int main(void)
{
		SOSC_init_8MHz();       /* Initialize system oscilator for 8 MHz xtal */
	  SPLL_init_160MHz();     /* Initialize SPLL to 160 MHz with 8 MHz SOSC */
	  NormalRUNmode_80MHz();  /* Init clocks: 80 MHz sysclk & core, 40 MHz bus, 20 MHz flash */
		LPTMR_init();						/*WODG time_init*/
		ivcu_poweron();
		wdgTps_init();
		uartInit(UART0,9600);
		uartIrqInit(UART0);
		uartInit(UART1,115200);
		uartIrqInit(UART1);
		Canbus0Init(500000);
		Canbus0IrqInit();
		Canbus1Init(500000);
		Canbus1IrqInit();//xuyao gai uart.c queueMaxSize
		int flag = 0;
		int i = 0;
		int LTE_status = mem909s_msmInit();
		int timeout = 0;
		char *AT_IPSENDEX="AT^IPSENDEX=2,1,\"7e010000000a00000000000a033131311203323232107e\"\n\r";
		char rdata[100]={0};
		int rLenNow=0;
		if(LTE_status == 0 )
			for(;;)
		{
		uartReadIrq(UART0,(unsigned char *)rdata,sizeof(rdata),&rLenNow);
			mem909s_memrecvdataserver();
			if(LTE_status == 0 ){
				if((timeout % 10000) == 0){
					timeout = 1;
					mem909s_msmsendserver(AT_IPSENDEX);
				}
				timeout++;
			}
			wdgTps_process();
			getReceivstatus();
//			type0mcu_receive_type1mcu();
//			if(Analysis_zynq_type0_flag == 0)
//			{
//				i++;
//				if(i % 100 == 0)
//					send_zynq_request_type0();
//			}
//			receive_zynq_type0(rdata,rLenNow);
			int ret = updata(rdata,rLenNow);
			if(ret == 1){ 
				UART_Reset(UART0);
				UART_Reset(UART1);
				Canbus0_irq_Reset();
				Canbus1_irq_Reset();
				disable_can0();
				disable_can1();
				LPTMR_Reset();
				jump2app = (bootloader_fun)*(uint32_t*)(UPDATA_START_ADDRESS + 4);
				jump2app();
			}
		}
	return 0;
}

