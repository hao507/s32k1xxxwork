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
void delay2(volatile int cycles)
{
    /* Delay function - do nothing for a number of cycles */
    while(cycles--);
}

int AT_Send(char *AT_Order,int flag,int *rData_offset) //AT_Order:Ҫ���͵�ָ��  falg����ͬ���շ�ʽ Mate_Start��ƥ�俪ʼ���ַ����� 0��ʾ����Ĭ�Ͽ�ͷƥ����ַ����Ƽ�����2����ʾ���������ַ��������н���
{
	char Mate_End[100]={'O','K'};	
	char Mate_Error[100]={'E','R','R','O','R'};
	char rData[100]={0};
	char rData_1[100]={0};
	char Mate_St[100]={0};	
	char recvBuf[100] = {0};
	int rLenNow = 0,recvNum_End,stat,recvNum;
	char *wData_Error = "send error\n\r";
	strcpy(Mate_St,AT_Order);
	strcat(Mate_St,"\r\n");
	int n;
	if(flag == 0)
	{
		while(1)
		{
			uartSendIrq(UART1,(unsigned char *)AT_Order,strlen(AT_Order),&rLenNow);
			//delay2(6000);
			if(UART1_SEND_OVER == uartSendStatusGet(UART1))
			{
				uartReadIrq(UART1,rData,sizeof(rData),&rLenNow);
				delay2(600000);
				if(rLenNow > strlen(AT_Order))//�����Ѿ����ܵ�����
				{
					for(int i = 0; i < rLenNow; i++)
					{
						if(strncmp(rData+i,AT_Order,strlen(AT_Order)) == 0)
						{
							stat =  1;
							recvNum = i;
						}
						if(strncmp(Mate_End,rData+i,strlen(Mate_End)) == 0 && stat == 1)
						{
							recvNum_End = i+4;
							strncpy(recvBuf,rData+recvNum,recvNum_End - recvNum);
							uartSendIrq(UART0,(unsigned char *)recvBuf,strlen(recvBuf),&recvNum_End);
							int return_num;
							return_num = judeg(AT_Order,recvBuf);
							return return_num;
						}
					}
					
				}	
			}
		}
	}
	if(flag == 1)
	{
		while(1)
		{
			uartSendIrq(UART1,(unsigned char*)AT_Order,strlen(AT_Order),&rLenNow);
		//	delay2(6000);
			if(UART1_SEND_OVER == uartSendStatusGet(UART1))
			{
				uartReadIrq(UART1,rData_1,sizeof(rData_1),&rLenNow);
		//		delay2(60000);
				if(rLenNow > strlen(AT_Order))//�����Ѿ����ܵ�����
				{
					strcpy(recvBuf,rData_1+*rData_offset);
					uartSendIrq(UART0,(unsigned char *)recvBuf,strlen(recvBuf),&recvNum_End);
			//		delay2(6000);
					
				}	
			*rData_offset = rLenNow;
			if(*rData_offset > 99)
			{
				*rData_offset = *rData_offset - 100;
			}
			return 0;
			}
		
		}
	}
}

		



