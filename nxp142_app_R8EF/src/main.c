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
		uartInit(UART0,9600);
		uartIrqInit(UART0);	/* Configure ports */
		uartInit(UART1,100000);
		uartIrqInit(UART1);
		Canbus0Init(500000);
		Canbus0IrqInit();
	/*!
	 * Infinite for:
	 * ========================
	 */
	unsigned char rData[50]={0};
	unsigned char buf[50]={0};
	int rLenNow = 0;
	char CanSendId = 0x12;
//	unsigned char sendData[16]={1,2,3};
	unsigned short data[16]={0};
	unsigned short data1[8]={0};
	char send[10] = {0x01,0x02};
	int sLenNow = 0;
	char cansend[2]={0};
	char *end = "\n\r";
	char SendID0 = 0x70;
	int  data2[100]={0};
			int j = 0;
  for(;;)
  {
		uartReadIrq(UART1, rData,sizeof(rData),&rLenNow);
		if(rLenNow >= 25){
			for(int i = 0; i < rLenNow;i++){
			if(rData[i] == 0x0F && rData[i+24] == 0x00){
				decode(rData,data,i);

				if((data[5] == 0xC8 || data[5] == 0x708 )&& (data[4] == 0x708 || data[4] == 0x3E8 || data[4] == 0x0c8) &&(data[8] == data[9]) && (data[15] == 0x400)){
					if(data[0] >= 1000){
						data[0] = data[0]*k1+b1;
					}
					else{
						data[0] = data[0]*k0+b0;
					}
//					//    val[0] = ((buff[ind + 1] | buff[ind + 2] << 8) & 0x07FF);
						if(data[1] >= 1000){
							data[1] = data[1]*k1+b1;
						}
						else if(data[1] < 1000) {
							data[1] = data[1]*k0+b0;
						}
						if(data[2] >= 1000){
							data[2] = data[2]*k1+b1;
						}
						else if(data[2] < 1000) {
							data[2] = data[2]*k0+b0;
						}
						if(data[3] >= 1000){
							data[3] = data[3]*k1+b1;
						}
						else if(data[3] < 1000) {
							data[3] = data[3]*k0+b0;
						}
						if(data[7] >= 1000){
							data[7] = data[7]*k1+b1;
						}
						else if(data[7] < 1000) {
							data[7] = data[7]*k0+b0;
						}
					if(data[0] != data1[0]){
							data1[0] = data[0];
						int cansendID = 0x010;
						cansend[0] = (data[0] >> 8)&0xFF;
						cansend[1] = data[0] & 0xFF;
						Canbus0Send(cansendID,(unsigned char *)cansend,2,CAN_STANDARD_MODE);
						delay(8000);
					}
					if(data[1] != data1[1]){
						int diff = data[1]-data1[1];
							data1[1] = data[1];
							int cansendID = 0x011;
							cansend[0] = (data[1] >> 8)&0xFF;
							cansend[1] = data[1] & 0xFF;
							Canbus0Send(cansendID,(unsigned char *)cansend,2,CAN_STANDARD_MODE);
						 	delay(8000);
					}
					if(data[2] != data1[2]){
							data1[2] = data[2];
							int cansendID = 0x012;
							cansend[0] = (data[2] >> 8)&0xFF;
							cansend[1] = data[2] & 0xFF;
							Canbus0Send(cansendID,(unsigned char *)cansend,2,CAN_STANDARD_MODE);
						  delay(8000);
						if(j < 100){
							data2[j] = data[2];
							j++;
						}
					}
					if(data[3] != data1[3]){
						data1[3] = data[3];
						int cansendID = 0x013;
						cansend[0] = (data[3] >> 8)&0xFF;
						cansend[1] = data[3] & 0xFF;
						Canbus0Send(cansendID,(unsigned char *)cansend,2,CAN_STANDARD_MODE);
						delay(8000);
					}
					if(data[4] != data1[4]){
						data1[4] = data[4];
						int cansendID = 0x14;
						short CH;
						if(data[4] == 0x708){
							CH = 0x7CF;
						}
						else if(data[4] == 0x3E8){
							CH = 0x5DB;
						}
						else if(data[4] == 0xC8){
							CH = 0x3E7;
						}
						cansend[0] =(CH >> 8)&0xFF;
						cansend[1] = CH & 0xFF;
						Canbus0Send(cansendID,(unsigned char *)cansend,2,CAN_STANDARD_MODE);
						delay(8000);
					}
					if(data[5] != data1[5]){
						data1[5] = data[5];
						int cansendID = 0x15;
						short CH;
						if(data[5] == 0x708){
							CH = 0x7CF;
						}
						else if(data[5] == 0xC8){
							CH = 0x3E7;
						}
						cansend[0] = CH >> 8;
						cansend[1] = CH & 0xFF;
						Canbus0Send(cansendID,(unsigned char *)cansend,2,CAN_STANDARD_MODE);
						delay(8000);
					}
					if(data[6] != data1[6]){
						data1[6] = data[6];
						int cansendID = 0x16;
						short CH;
						if(data[6] == 0x708){
							CH = 0x7CF;
						}
						else if(data[6] == 0x3E8){
							CH = 0x5DB;
						}
						else if(data[6] == 0xC8){
							CH = 0x3E7;
						}
						cansend[0] = CH >> 8;
						cansend[1] = CH & 0xFF;
						Canbus0Send(cansendID,(unsigned char *)cansend,2,CAN_STANDARD_MODE);
						delay(8000);
					}
					if(data[7] != data1[7]){
						data1[7] = data[7];
						int cansendID = 0x17;
						cansend[0] = data[7] >> 8;
						cansend[1] = data[7] & 0xFF;
						Canbus0Send(cansendID,(unsigned char *)cansend,2,CAN_STANDARD_MODE);
						delay(8000);
					}
					memset(rData,1,50);
				}
				break;
				}
			}
		}
	}
}


