#include "S32K142.h" /* include peripheral declarations S32K144 */
#include "clocks_and_modes.h"
#include "gpio.h"
#include "usertype.h"
#include "uart.h"
#include "updata.h"
#include <string.h>
#include "mem909s.h"
#include "Type0_MUC_Test.h"
#include "wdgTps.h"
#include "canbus.h"
#include "timer.h"

void wdog_test_intt()
{
	LPTMR_init();
	wdgTps_init();
}
void feed_wdog()
{
	wdgTps_process();
}


void canbus_send_test()
{
		unsigned char wdata[] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08};
			Canbus0Send(Can_Send_Id,(unsigned char *)wdata,strlen((char *)wdata),CAN_STANDARD_MODE);
}

void canbuf_read_test()
{
		int Can_Read_Id;
		unsigned char Read_data[200]={0};
		int queueLen;
		canbus0ReadIrq((int *)&Can_Read_Id,Read_data,&queueLen);
		if(queueLen > 0)
		{
				
		}
}