#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int judeg(char *order ,char *respond)
{
	char *AT = "AT\n\r";//��ѯģ���Ƿ����
	char *AT_CPIN = "AT+CPIN?\n\r";//���PIN���Ƿ�׼������
	char *AT_CSQ = "AT+CSQ\n\r";//��ѯ�ź�����	
	char *AT_CREG = "AT+CREG?\n\r";//��ѯ��ǰ����ע��״����
	char *AT_COPS = "AT+COPS?\n\r";
	char *AT_COPS_ = "AT+COPS=?\n\r";//��ѯ ME ����עnihao��ĵ�ǰ״̬��
	char *AT_COPS_0 = "AT+COPS=0\n\r";//�Զ���������
	char *AT_CGATT = "AT+CGATT?\n\r";
	char *Call = "ATD15773068148;";//���� 15773068148 �绰
	char *AT_SYSINFOEX = "AT^SYSINFOEX\n\r";
	char *AT_CMGF="AT+CMGF=1\n\r";
	char *AT_CSMP = "AT+CSMP=,,0,8\n\r";
	char *AT_CSCS="AT+CSCS=\"UCS2\"\n\r";
	if(strcmp(order,AT) == 0)
	{
		if(strncmp(respond+6,"OK",2) == 0)
				return 1;
			return -1;
	}
	else if(strcmp(order,AT_CPIN) == 0)
	{
		if(strncmp(respond+19,"READY",5) == 0)
				return 1;
			return -1;
	}
	else if(strcmp(order,AT_CSQ) == 0)
	{
		int num_1= *(respond+16)-0;
		int num_2= *(respond+17)-0;
		if(num_1 >= 1 && num_2 >= 8)
			if(strncmp(respond+25,"OK",2) == 0)
				return 1;
			return -1;
	}
	else if(strcmp(order,AT_CMGF) == 0)
	{
		if(strncmp(respond+13,"OK",2) == 0)
				return 1;
			return -1;
	}
	else if(strcmp(order,AT_CSMP) == 0)
	{
		if(strncmp(respond+17,"OK",2) == 0)
				return 1;
			return -1;
	}
	else if(strcmp(order,AT_CSCS) == 0)
	{
		if(strncmp(respond+18,"OK",2) == 0)
				return 1;
			return -1;
	}
}