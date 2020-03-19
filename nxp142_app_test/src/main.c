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
#include "TYPE2_mcu_Test.h"

#pragma comment(lib, "libprotobuf.lib")

#define UPDATA_START_ADDRESS  0x00000000
#define TYPE 2
/*
	type0   0 
	type1   1
	type2   2
	type1.1 3
*/





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
#if TYPE != 3
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
#endif
#if TYPE ==	3
void ivcu_poweron()
{
 int i = 0;
 GpioSetDirection(PORT_D, 0, PORT_OUT);
 GpioSetDirection(PORT_D, 1, PORT_OUT);
 GpioSetDirection(PORT_D, 3, PORT_OUT);
 GpioSetDirection(PORT_D, 4, PORT_OUT);
 GpioSetDirection(PORT_D, 5, PORT_OUT);
 GpioSetDirection(PORT_E, 9, PORT_OUT);
 GpioSetDirection(PORT_E, 10, PORT_OUT);
 GpioSetDirection(PORT_E, 11, PORT_OUT);

 GpioSetVal(PORT_D, 0, PORT_HIGH);
 GpioSetVal(PORT_D, 3, PORT_HIGH);
 GpioSetVal(PORT_D, 4, PORT_HIGH);

 GpioSetVal(PORT_E, 2, PORT_HIGH);
 GpioSetVal(PORT_E, 3, PORT_HIGH);
 
 GpioSetVal(PORT_D, 5, PORT_HIGH);//XAVIER_PWR_EN_3V3
 GpioSetVal(PORT_E, 11, PORT_HIGH);//MCU_XAVIER_POWER_BTN_N 
 delay(80000);//10ms
 wdgTps_process();
  GpioSetVal(PORT_E, 11, PORT_LOW);//MCU_XAVIER_POWER_BTN_N 
 delay(20000);
 GpioSetVal(PORT_E, 9, PORT_HIGH);//MCU_XAVIER_VIN_PWR_ON
 wdgTps_process();
 for(i=0; i< 8;i++){
  delay(80000);
  wdgTps_process();
 }
 wdgTps_process();
 GpioSetVal(PORT_E, 10, PORT_HIGH);//MCU_XAVIER_MODULE_POWER_ON_3V3
 //delay(80000*2);//10ms
 wdgTps_process();
 GpioSetVal(PORT_E, 11, PORT_HIGH);//MCU_XAVIER_POWER_BTN_N 
 delay(80000);//10ms
 GpioSetVal(PORT_D, 1, PORT_HIGH);//XAVIER_RESET
}
#endif
int main(void)
{
//			GpioSetVal(PORT_D, 15, PORT_HIGH);
//		delay(8000);
//		GpioSetVal(PORT_D, 15, PORT_LOW);

		SOSC_init_8MHz();       /* Initialize system oscilator for 8 MHz xtal */
	  SPLL_init_160MHz();     /* Initialize SPLL to 160 MHz with 8 MHz SOSC */
	  NormalRUNmode_80MHz();  /* Init clocks: 80 MHz sysclk & core, 40 MHz bus, 20 MHz flash */
		LPTMR_init();						/*WODG time_init*/
		wdgTps_init();
		wdgTps_process();
		ivcu_poweron();
//		GpioSetVal(PORT_D, 15, PORT_HIGH);
//		delay(8000);
//		GpioSetVal(PORT_D, 15, PORT_LOW);
		wdgTps_process();
		int time = 0;
		char LTE_rData[200]={0};
		char rdata[100]={0};
		int rLenNow=0;
		int ret = -1;
#if TYPE == 0
		type0_MCU_test_init();
#endif
		#if TYPE == 1 || TYPE == 3
		TYPE1_MCU_test_init();
#endif
#if TYPE == 2
		TYPE2_MCU_test_init();
#endif
			for(;;)
		{
#if TYPE == 0
		uartReadIrq(UART0,(unsigned char *)rdata,sizeof(rdata),&rLenNow);
#endif
#if TYPE == 1 || TYPE == 2 || TYPE == 3
		uartReadIrq(UART1,(unsigned char *)rdata,sizeof(rdata),&rLenNow);
#endif
			wdgTps_process();
#if TYPE == 0
		type0_MCU_Automatic_testing_Start(rdata,rLenNow);
#endif
#if TYPE == 1 || TYPE == 3
		TYPE1_MCU_Automatic_testing_Start(rdata,rLenNow);
#endif
#if TYPE == 2
		TYPE2_MCU_Automatic_testing_Start(rdata,rLenNow);
#endif
			wdgTps_process();
			int ret = updata(rdata,rLenNow);
			if(ret == 1){
				UART_Reset(UART0);
				UART_Reset(UART1);
				Canbus0_irq_Reset();
				Canbus1_irq_Reset();
				disable_can0();
				disable_can1();
				for(;;){
					int val = wdgTps_process();
					if(val == 1)
						break;
				}
				LPTMR_Reset();
				wdgTps_process();
				jump2app = (bootloader_fun)*(uint32_t*)(UPDATA_START_ADDRESS + 4);
				jump2app();
			}
			time++;
		}
	return 0;
}

