#include "S32K142.h" /* include peripheral declarations S32K144 */
#include "clocks_and_modes.h"
#include "gpio.h"
#include "usertype.h"
#include "uart.h"
#include "updata.h"
#include "timer.h"
#include <string.h>

#define APP_START_ADDRESS  0x00004000



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
#if 0
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
	GpioSetDirection(PORT_E, 5, PORT_OUT);

	GpioSetVal(PORT_D, 0, PORT_HIGH);
	//GpioSetVal(PORT_D, 3, PORT_HIGH);
	GpioSetVal(PORT_E, 2, PORT_HIGH);
	GpioSetVal(PORT_E, 3, PORT_HIGH);
	GpioSetVal(PORT_E, 5, PORT_HIGH);
}
#else
void ivcu_poweron()
{
 GpioSetDirection(PORT_D, 0, PORT_OUT);
 GpioSetDirection(PORT_D, 1, PORT_OUT);
 GpioSetDirection(PORT_D, 2, PORT_OUT);
 GpioSetDirection(PORT_D, 3, PORT_OUT);
 GpioSetDirection(PORT_D, 4, PORT_OUT);
 GpioSetDirection(PORT_D, 5, PORT_OUT);
 GpioSetDirection(PORT_D, 6, PORT_OUT);
 GpioSetDirection(PORT_D, 16, PORT_OUT);
GpioSetDirection(PORT_E, 10, PORT_OUT);
 GpioSetDirection(PORT_E,8,PORT_OUT);   //MCU Dormancy control
//GpioSetDirection(PORT_E, 11, PORT_OUT);  

 GpioSetVal(PORT_D, 0, PORT_HIGH);
 GpioSetVal(PORT_D, 1, PORT_HIGH);
 GpioSetVal(PORT_D, 2, PORT_HIGH);
 GpioSetVal(PORT_D, 3, PORT_HIGH);
 GpioSetVal(PORT_D, 4, PORT_HIGH);
 GpioSetVal(PORT_D, 5, PORT_HIGH);
 GpioSetVal(PORT_D, 6, PORT_HIGH);
// GpioSetVal(PORT_D, 0, PORT_LOW);
// GpioSetVal(PORT_D, 1, PORT_LOW);
// GpioSetVal(PORT_D, 2, PORT_LOW);
// GpioSetVal(PORT_D, 3, PORT_LOW);
// GpioSetVal(PORT_D, 4, PORT_LOW);
// GpioSetVal(PORT_D, 5, PORT_LOW);
// GpioSetVal(PORT_D, 6, PORT_LOW);
 GpioSetVal(PORT_E,8, PORT_LOW);
GpioSetVal(PORT_E, 10, PORT_LOW);
 delay(80000);
 GpioSetVal(PORT_D, 1, PORT_LOW);//
 GpioSetVal(PORT_D, 2, PORT_LOW);//
 delay(80000);
 GpioSetVal(PORT_D, 1, PORT_HIGH);
 GpioSetVal(PORT_D, 2, PORT_HIGH);
//GpioSetVal(PORT_E, 10, PORT_HIGH);
//GpioSetVal(PORT_E, 11, PORT_HIGH);
}
void xavier_poweron()
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
//void JumpToUserApplication( unsigned int userSP,  unsigned int userStartup)
//{
//	/* Check if Entry address is erased and return if erased */
//	if(userSP == 0xFFFFFFFF){
//		return;
//	}

//	/* Set up stack pointer */
//	__asm("msr msp, r0");
//	__asm("msr psp, r0");

//	/* Relocate vector table */
//	S32_SCB->VTOR = (uint32_t)APP_START_ADDRESS;

//	/* Jump to application PC (r1) */
//	__asm("mov pc, r1");
//}
int sendStatus = 0;
int main(void)
{
	  int ret = 0, slen = 0, rlen = 0;
		int timeNum = 0, num;
		unsigned char wData[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	  WDOG_disable();
		SOSC_init_8MHz();       /* Initialize system oscilator for 8 MHz xtal */
	  SPLL_init_160MHz();     /* Initialize SPLL to 160 MHz with 8 MHz SOSC */
	  NormalRUNmode_80MHz();  /* Init clocks: 80 MHz sysclk & core, 40 MHz bus, 20 MHz flash */
	  
		GpioInit();
		LPIT0_noIrq_init();//LPTMR_init();//
		wdgTps_init();
		flash_init();
#if TYPE != 3
		ivcu_poweron();
#else
		xavier_poweron();
#endif
#if 1
#if TYPE == 0
    uartInit(UART0, 9600);
#endif
#if TYPE == 1|| TYPE==2 || TYPE == 3
    uartInit(UART1, 9600);
#endif
		wdgTps_noIrq_process1();
		updata_handle(updata_jump, wData, 0, 0);
		while(1){
#if 1			
			timeNum++;
			wdgTps_noIrq_process1();//wdgTps_process();
			updata_recv();
			if(timeNum > 20 && updata_status < 0){
				timeNum = 0;
				break;
			}
		 if(updata_status == updata_end){
				break;
			}
#else
			wdgTps_noIrq_process1();
#endif
		}
#else
		uartInit(UART0, 9600);
		uartInit(UART1, 115200);
//    char *wData_ati = "ATI\n\r";
	  char *wData_pir = "AT+IPR=?\n\r";
		//char *wData_cpin = "AT+CPIN?\n\r";
		unsigned char rData[10] ={0};
//	
	  for(;;)
		{
//			  uartSendBlock(UART1, wData_pir, strlen(wData_pir), 10);
//			int ret = uartReadBlock(UART1, rData, sizeof(rData), 10);
//			rData[0] = rData[0];
//			if(ret > 0)
//			  uartSendBlock(UART0, rData, sizeof(rData), 10);
				uartSendBlock(UART0, wData, sizeof(wData), 10);
			
//			if(sendStatus == 0)
//				ret = uartSendIrq(UART1, (unsigned char*)wData_ati, strlen(wData_ati), &slen);
//			if(ret == UART1_SEND_OVER && sendStatus == 0)
//				sendStatus = 1;
//			if(sendStatus==1)
//				ret = uartSendIrq(UART1, (unsigned char*)wData_pir, strlen(wData_pir), &slen);
//			if(ret == UART1_SEND_OVER && sendStatus == 1)
//				sendStatus = 2;
//			if(sendStatus==2)
//				ret = uartSendIrq(UART1, (unsigned char*)wData_pir, strlen(wData_pir), &slen);
//			if(ret == UART1_SEND_OVER && sendStatus == 2)
//				sendStatus = 1;
			//uartReadIrq(UART0, rData, sizeof(rData), &rlen);
		}
#endif
			//uart reset ,the app uart can not send
			//UART_pin_settings_reset(UART0);
			//UART_Reset(UART0);
		  //LPTMR_Reset();
			for(;;){
				int val = wdgTps_noIrq_process1();
				if(val == 1)
					break;
			}
			LPIT0_noIrq_uninit();
			jump2app = (bootloader_fun)*(uint32_t*)(APP_START_ADDRESS + 4);
			jump2app();
			//JumpToUserApplication(*((uint32_t*)APP_START_ADDRESS), *((uint32_t*)(APP_START_ADDRESS + 4)));
	for(;;)
	
	
	return 0;
}
