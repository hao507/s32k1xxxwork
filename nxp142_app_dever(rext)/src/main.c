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
#define TYPE 3
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
float k1 = 0.62375;
float b1 = 876.25;
float k0 = 0.62625;
float b0 = 873.75;
void decode(unsigned char *buff,unsigned short *val,int ind)
{
    val[0] = ((buff[ind + 1] | buff[ind + 2] << 8) & 0x07FF);
    val[1] = ((buff[ind + 2] >> 3 | buff[ind + 3] << 5) & 0x07FF);
    val[2] = ((buff[ind + 3] >> 6 | buff[ind + 4] << 2 | buff[ind + 5] << 10) & 0x07FF);
    val[3] = ((buff[ind + 5] >> 1 | buff[ind + 6] << 7) & 0x07FF);
    val[4] = ((buff[ind + 6] >> 4 | buff[ind + 7] << 4) & 0x07FF);
    val[5] = ((buff[ind + 7] >> 7 | buff[ind + 8] << 1 | buff[ind + 9] << 9) & 0x07FF);
    val[6] = ((buff[ind + 9] >> 2 | buff[ind + 10] << 6) & 0x07FF);
    val[7] = ((buff[ind + 10] >> 5 | buff[ind + 11] << 3) & 0x07FF);
    val[8] = ((buff[ind + 12] | buff[ind + 13] << 8) & 0x07FF);
    val[9] = ((buff[ind + 13] >> 3 | buff[ind + 14] << 5) & 0x07FF);
    val[10] = ((buff[ind + 14] >> 6 | buff[ind + 15] << 2 | buff[ind + 16] << 10) & 0x07FF);
    val[11] = ((buff[ind + 16] >> 1 | buff[ind + 17] << 7) & 0x07FF);
    val[12] = ((buff[ind + 17] >> 4 | buff[ind + 18] << 4) & 0x07FF);
    val[13] = ((buff[ind + 18] >> 7 | buff[ind + 19] << 1 | buff[ind + 20] << 9) & 0x07FF);
    val[14] = ((buff[ind + 20] >> 2 | buff[ind + 21] << 6) & 0x07FF);
    val[15] = ((buff[ind + 21] >> 5 | buff[ind + 22] << 3) & 0x07FF);
}
	int num = 0;
#define SBUS_START 0x0F
int main(void)
{
  uint32_t counter = 0;

	/*!
	 * Initialization:
	 * =======================
	 */

  WDOG_disable();		   /* Disable WDOG */
  SOSC_init_8MHz();        /* Initialize system oscillator for 8 MHz xtal */
  SPLL_init_160MHz();      /* Initialize SPLL to 160 MHz with 8 MHz SOSC */
  NormalRUNmode_80MHz();   /* Init clocks: 80 MHz sysclk & core, 40 MHz bus, 20 MHz flash */
	LPTMR_init();
//	uartInit(UART0,9600);
//	uartIrqInit(UART0);	/* Configure ports */
//	uartInit(UART1,100000);
//	uartIrqInit(UART1);
//	CanbusInit(CANBUS0,500000);
//	CanbusIrqInit(CANBUS0);
//	GpioInit();
//	GpioSetDirection(PORT_B,2,PORT_OUT);
//	GpioSetVal(PORT_B,2,0);
	/*!
	 * Infinite for:
	 * ========================
	 */
  for(;;)
  {
		num = LPTMR_CNR_COUNTER(1);
		delay(8000);
		num = LPTMR_CNR_COUNTER(0);
	}
}


