#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include "uart.h"
#include "S32K142.h" /* include peripheral declarations S32K144 */
#include "clocks_and_modes.h"
#include "gpio.h"
#include "usertype.h"
#include "uart.h"
#include "AT_Send.h"
#include "AT_init.h"
void delay3(volatile int cycles)
{
    /* Delay function - do nothing for a number of cycles */
    while(cycles--);
}
int stat = 0;
int Accept(char *Phone_number,char *Content)
{//+CMGL: 1,"REC UNREAD","+8617382092737",,"19/08/01,14:00:37+32"
//1
	char *AT_CMGF="AT+CMGF=1\n\r";
	char *AT_CMGD="AT+CMGD=1,4\n\r";
	char rData[200] = {0};
	char recvbuff[200] = {0};
	
	int ret = 0,rLenNow = 0;
	char *r_1 = "1\n\r";
	char *r_2 = "2\n\r";
	char *r_3 = "3\n\r";
	char *r_4 = "4\n\r";
	char *AT_CMGL="AT+CMGL=\"REC UNREAD\"\n\r";
		AT_Send(AT_CMGF,0,&rLenNow);
		uartSendIrq(UART1,(unsigned char *)AT_CMGD,strlen(AT_CMGD),&rLenNow);
	while(1)
	{
		uartSendIrq(UART1,(unsigned char *)AT_CMGL,strlen(AT_CMGL),&rLenNow);
		if(UART1_SEND_OVER == uartSendStatusGet(UART1))
		{
			uartReadIrq(UART1,rData,sizeof(rData),&rLenNow);
			if(rLenNow > 0)
			{
				for(int i = 0 ;i < 199 ;i++)
				{
					if(strncmp(rData+i,"+CMGL:",6) == 0)
					{
							stat = 1;
					}
					if(strncmp(rData+i,Phone_number,strlen(Phone_number)) == 0 && stat == 1)
					{
						stat = 2;
					}
					if(strncmp(rData+i,Content,strlen(Content)) == 0 && stat == 2)
					{
						stat = 3;
						memset(rData,0,sizeof(rData));
						for(i = 0;i < 100;i++)
						{
							uartSendIrq(UART1,(unsigned char *)AT_CMGL,strlen(AT_CMGL),&rLenNow);
						}
						uartSendIrq(UART1,(unsigned char *)AT_CMGD,strlen(AT_CMGD),&rLenNow);	
							return 1;
						
					}
				}
			}
		}
	}	
}