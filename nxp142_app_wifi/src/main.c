#include "S32K142.h" /* include peripheral declarations S32K144 */
#include "clocks_and_modes.h"
#include "gpio.h"
#include "usertype.h"
#include "uart.h"
#include <string.h>
#include "canbus.h"
#include "timer.h"
#include "spi.h"
#include <stdint.h>
#pragma comment(lib, "libprotobuf.lib")
#include "main.h"




#define USE_WEP (0) /*< Set to (1) to use WEP, and (0) to use OPEN */

/** AP mode Settings */
#define MAIN_WLAN_SSID "DEMO_AP" /* < SSID */
#if USE_WEP
#define MAIN_WLAN_AUTH M2M_WIFI_SEC_WEP /* < Security manner */
#define MAIN_WLAN_WEP_KEY "1234567890"  /* < Security Key in WEP Mode */
#define MAIN_WLAN_WEP_KEY_INDEX (0)
#else
#define MAIN_WLAN_AUTH M2M_WIFI_SEC_OPEN /* < Security manner */
#endif
#define MAIN_WLAN_CHANNEL (6) /* < Channel number */
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
int main(void)
{
	SOSC_init_8MHz();       /* Initialize system oscilator for 8 MHz xtal */
	SPLL_init_160MHz();     /* Initialize SPLL to 160 MHz with 8 MHz SOSC */
	NormalRUNmode_80MHz();  /* Init clocks: 80 MHz sysclk & core, 40 MHz bus, 20 MHz flash */
	GpioInit();
	while(1){
		
	}
}
