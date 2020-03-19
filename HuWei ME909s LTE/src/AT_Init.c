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
int rData_offset = 0;
int flag = 0;
void delay1(volatile int cycles)
{
    /* Delay function - do nothing for a number of cycles */
    while(cycles--);
}
void AT_init()
{
	char *AT = "AT\n\r";//��ѯģ���Ƿ����
	char *AT_CPIN = "AT+CPIN?\n\r";//���PIN���Ƿ�׼������
	char *AT_CSQ = "AT+CSQ\n\r";//��ѯ�ź�����	
	char *AT_CREG = "AT+CREG?\n\r";//��ѯ��ǰ����ע��״����
	char *AT_COPS = "AT+COPS?\n\r";
	char *AT_COPS_ = "AT+COPS=?\n\r";//��ѯ ME ����ע��ĵ�ǰ״̬��
	char *Call = "ATD17382092737;";//���� 15773068148 �绰
	int rLenNow;
	char *AT_CMGF= "AT+CMGF=1\n\r";
	char *AT_CSMP = "AT+CSMP=,,0,8\n\r";
	char *AT_CSCS= "AT+CSCS=\"UCS2\"\n\r";
	char *AT_CSCA= "AT+CSCA=\"8613800731500\",145\n\r";
	char *AT_CMGS = "AT+CMGS=\"00310037003300380032003000390032003700330037\"\n\r";
	char *AT_CMGS_2 = "4F60597D";
	char *at0 = "ATZ0";
	char 	ch_2 = 0x1A;
	int ret = -1 ;
	if(flag == 0)
	{
		flag = 1;
//		AT_Send(at0,1,&rData_offset);
		AT_Send(AT_CMGF,0,&rData_offset);
		AT_Send(AT_CSMP,0,&rData_offset);
		AT_Send(AT_CSCA,0,&rData_offset);
		AT_Send(AT_CSCS,0,&rData_offset);
	}
	AT_Send(AT_CMGS,1,&rData_offset);
	//delay1(50000);
	AT_Send(AT_CMGS_2,1,&rData_offset);
	//delay1(50000);
	AT_Send(&ch_2,1,&rData_offset);
//delay1(1000000);
}