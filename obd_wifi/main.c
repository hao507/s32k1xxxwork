#include "S32K142.h" /* include peripheral declarations S32K144 */
#include "clocks_and_modes.h"
#include "gpio.h"
#include "usertype.h"
#include "uart.h"
#include "printf.h"
#include "spi.h"
#include "timer.h"
#include "winc_nxp.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "main.h"
#include "winc1500_api.h"
#include "wf_socket.h"
#include "wf_spi.h"
#include <string.h>
#include "flash_driver.h"
#include "pcc_hal.h"
#include <stdint.h>
#include "demo_config.h"


#define STRING_EOL    "\r\n"
#define STRING_HEADER "-- WINC1500 AP mode example --"STRING_EOL \
	"-- "BOARD_NAME " --"STRING_EOL	\
	"-- Compiled: "__DATE__ " "__TIME__ " --"STRING_EOL

/** Message format definitions. */
typedef struct s_msg_wifi_product {
	uint8_t name[9];
} t_msg_wifi_product;

typedef struct s_msg_wifi_product_main {
	uint8_t name[9];
} t_msg_wifi_product_main;

/** Message format declarations. */
static t_msg_wifi_product msg_wifi_product = {
	.name = MAIN_WIFI_M2M_PRODUCT_NAME,
};

static t_msg_wifi_product_main msg_wifi_product_main = {
	.name = MAIN_WIFI_M2M_PRODUCT_NAME,
};

uint32_t g_winc_logstart;

/** Socket for Rx */
static SOCKET tx_socket = -1;

/** Test buffer */
static uint8_t gau8SocketTestBuffer[MAIN_WIFI_M2M_BUFFER_SIZE] = {0};

int nm_clkless_wake(void)
{
	int ret;
	uint32_t reg	= nm_read_reg(0x1);
	/* Set bit 1 */
//	ret = nm_write_reg(0x1, reg | (1 << 1));
//	// Check the clock status
//	ret = nm_read_reg_with_ret(clk_status_reg_adr, &clk_status_reg);
//	// Tell Firmware that Host waked up the chip
//	ret = nm_write_reg(WAKE_REG, WAKE_VALUE);
	return ret;
}
/**
 * \brief Main application function.
 *
 * \return program return value.
 */
#if 1
int main(void)
{
	struct sockaddr_in addr;
	int8_t ret;
	volatile uint32_t send_dly = 0;
	unsigned char wData[10] = {0xAA, 0x55, 0xE8, 0x24, 0xBC, 0x33, 0x42, 0x43, 0x52, 0x53};

	tstrM2MAPConfig strM2MAPConfig;

	/* Initialize the board. */
	SOSC_init_8MHz();       /* Initialize system oscilator for 8 MHz xtal */
	SPLL_init_160MHz();     /* Initialize SPLL to 160 MHz with 8 MHz SOSC */
	NormalRUNmode_80MHz();  /* Init clocks: 80 MHz sysclk & core, 40 MHz bus, 20 MHz flash */

	GpioInit();
	GpioSetDirection(PORT_B,0,PORT_OUT);
	SpiInitMaster(8);
	
	uartInit(UART0, 9600);

	g_winc_logstart = 0;

	/* Initialize the BSP. */
	nm_bsp_init();
	m2mStub_time_init();

	/* Initialize Wi-Fi driver with data and status callbacks. */
	m2m_wifi_init();
	GpioSetVal(PORT_B,0,1);
	//nm_clkless_wake();
//	g_winc_logstart = 1;

	while(1){

		ApplicationTask();
		m2m_wifi_task();
		
	}	
	return 0;
}
#else
int main(void)
{
	  int ret = 0, slen = 0, rlen = 0;
		int timeNum = 0, num;
		unsigned char wData[10] = {0xAA, 0x55, 0xE8, 0x24, 0xBC, 0x33, 0x42, 0x43, 0x52, 0x53};
		SOSC_init_8MHz();       /* Initialize system oscilator for 8 MHz xtal */
	  SPLL_init_160MHz();     /* Initialize SPLL to 160 MHz with 8 MHz SOSC */
	  NormalRUNmode_80MHz();  /* Init clocks: 80 MHz sysclk & core, 40 MHz bus, 20 MHz flash */
	  GpioInit();
		SpiInitMaster(8);

	
		uartInit(UART0, 115200);
		g_winc_logstart = 0;

		/* Initialize the BSP. */
		nm_bsp_init();
		m2mStub_time_init();

		/* Initialize Wi-Fi driver with data and status callbacks. */
		m2m_wifi_init();		
		//spi_cmd1(0xca, 0xe824, 0, 4, 0);
		
		while(1){
				SpiSend (wData, 2, 10);
			  int timeOut = 8000*5;
			  while(timeOut--);
		}
}
#endif
