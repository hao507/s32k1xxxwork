#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "uart.h"
#include "usertype.h"
#include "canbus.h"
#include "wdgTps.h"
#include "mem909s.h"
//HEAD		ID			????	data		data		ROLLCNT  CRT  	 END4
//0X55 66	1BYTE		1BYTE		   							 0~255	          0X0D
// 0    1  2        3        4       5      6         7       8

#define TYPE2_endOkFind 1
#define TYPE2_endErrorFind 2
#define TYPE2_none 5
#define TYPE2_sendFail 3
#define TYPE2_beginFind 35


#define TYPE2_Read_Head_0 0x55
#define TYPE2_Read_Head_1 0x66
#define TYPE2_Send_Head_0 0x66
#define TYPE2_Send_Head_1 0x55
#define TYPE2_TAIL 0x0d


int TYPE2_rLenNow;
unsigned char TYPE2_Send_ROLLCNT = 0x00;
unsigned char TYPE2_Read_ROLLCNT = 0x00;
unsigned char TYPE2_Send_ROLLCNT_MAX = 0xFF;
unsigned char TYPE2_Read_ROLLCNT_MAX = 0XFF;
unsigned char TYPE2_rData[60]={0}; 
int TYPE2_Dog_flag = 0;
int TYPE2_ID_Start;

unsigned char TYPE2_uartsend_error[1]={0x01};
unsigned char TYPE2_uartsend_ok[1]={0x02};
int TYPE2_send_len = 0;
int TYPE2_LTE_status = -1;
int TYPE2_Can_read();
int TYPE2_Canread_statust0 = 0;
int TYPE2_Canread_statust1 = 0;
int TYPE2_TYPE2_Can_read_Id;
unsigned char TYPE2_Read_data0[200]={0};
unsigned char TYPE2_Read_data1[200]={0};
unsigned char TYPE2_send_TYPE2_buf[24]={0};
int TYPE2_LTE_init_flag = 0;
int TYPE2_MCU_test_init()
{
	wdgTps_init();
	uartInit(UART0,9600);
	uartIrqInit(UART0);
	uartInit(UART1,9600);
	uartIrqInit(UART1);
	CanbusInit(CANBUS0,500000);
	CanbusIrqInit(CANBUS0);
	CanbusInit(CANBUS1,500000);
	CanbusIrqInit(CANBUS1);
	TYPE2_Dog_flag = 1;
}
int TYPE2_flag_1 = 0;
int TYPE2_MCU_test_LTE();
int TYPE2_MCU_test_Send(int id,int len_data,unsigned char *data);
int TYPE2_MCU_test_Canbus();
int TYPE2_MCU_test_WDOG();
void TYPE2_MCU_test_Power_Management();
int TYPE2_MCU_test_Analysis(int rdatalen);

void TYPE2_MCU_Automatic_testing_Start(char *rdata ,int rLenNow)
{
	if(TYPE2_flag_1 == 0)
	{
		TYPE2_Canread_statust0 = 1;
		TYPE2_flag_1=1;
	}
		wdgTps_process();
	if(TYPE2_Canread_statust0 != 0 || TYPE2_Canread_statust1 != 0){
		TYPE2_Can_read();
	}
	int timeout = 10000,i;
	int flag_f = 0;
	unsigned char buf [40] ={0};
	TYPE2_rLenNow = rLenNow;
	for(int i = 0; i< rLenNow;i++){
		TYPE2_rData[i] = rdata[i];
	}
		for(int i= 0;i < TYPE2_rLenNow;i++)
		{
			if(TYPE2_rData[i] == TYPE2_Read_Head_0 && TYPE2_rData[i+1] == TYPE2_Read_Head_1){
				flag_f = 1;
			}
			if(TYPE2_rData[i] == TYPE2_TAIL && flag_f == 1)
			{
				TYPE2_MCU_test_Analysis(TYPE2_rLenNow);
				uartReadIrqReset(UART1);
				memset(TYPE2_rData,0,sizeof(TYPE2_rData));
				memset(rdata,0,rLenNow);
				break;
			}
		}
		memset(TYPE2_rData,0,sizeof(TYPE2_rData));
	if(TYPE2_Canread_statust0 != 0 || TYPE2_Canread_statust1 != 0){
		TYPE2_Can_read();
	}
}
int TYPE2_MCU_test_Analysis(rdatalen)
{	
	wdgTps_process();
	unsigned char crt = 0x00;
	unsigned char Read_Crt = 0x00;
	char data[20]={0};
	int read_data_len = 0;
	int data_strat=0;
	int readover_falg = 0;
	for(int i = 0;i < rdatalen;i++){
		if(TYPE2_rData[i] == TYPE2_Read_Head_0){
			
			data_strat = i;
			if(TYPE2_rData[data_strat+3] == 0){
				read_data_len = 7;
				break;
			}
			else if(TYPE2_rData[data_strat+3] == 2){
				read_data_len = 9;
				break;
			}
		}
	}
	crt = TYPE2_rData[data_strat] ^ TYPE2_rData[data_strat+1];
	for(int i = data_strat+2;i < data_strat+read_data_len-2;i++){
		crt = crt ^ TYPE2_rData[i];
	}
	if(crt == TYPE2_rData[data_strat+read_data_len-2]){
		TYPE2_ID_Start = data_strat+2;
		char ID = TYPE2_rData[data_strat+2];
			switch(ID){
			case 1:{
				TYPE2_MCU_test_Send(1,1,TYPE2_uartsend_ok);
				break;                
			}
			case 2:{
				TYPE2_MCU_test_Canbus();
				break;
			}
			case 3:{
				TYPE2_MCU_test_WDOG();
				break;
			}
			case 4:{
			TYPE2_MCU_test_Power_Management();
				break;
			}
			case 5:{
				TYPE2_MCU_test_LTE();
				break;
			}
		}
	}
	else{
		return -1;
	}
}
int TYPE2_Can_Send_Id_0 = 0x123;
int TYPE2_Can_Send_Id_1 = 0x234;
int TYPE2_send_flag_0 = 0;
int TYPE2_send_flag_1 = 0;

int TYPE2_MCU_test_Canbus()
{
		wdgTps_process();
	unsigned char data1 = TYPE2_rData[TYPE2_ID_Start+2];
	unsigned char data2 = TYPE2_rData[TYPE2_ID_Start+3];
	unsigned char TYPE2_buf[24]={0};
	int queueLen,tiomout = 10000;
	int flag_0 = 0;
	int TYPE2_flag_1 = 0;

	switch(data1){
		case 0:{
			TYPE2_Canread_statust0 = 0;
			TYPE2_send_flag_0 = 0;
			break;
		} 
		case 1:{
				TYPE2_Canread_statust0 = 1;
				TYPE2_send_flag_0 = 0;
				break;
		}
		case 2:{
			TYPE2_send_flag_0 = 1;
			TYPE2_Canread_statust0 = 0;
			break;
		}
		 default:{
			 if(data1 == 3)
			 {
				 TYPE2_Canread_statust0 = 1;
				 TYPE2_send_flag_0 =1;
			 }
			 break;
		}
	}
	
		switch(data2){
		case 0:{
			TYPE2_send_flag_1 = 0;
			TYPE2_Canread_statust1 = 0;
			break;
		} 
		case 1:{
			TYPE2_Canread_statust1 = 1;
			TYPE2_send_flag_1 = 0;
			break;
		}
		case 2:{
			TYPE2_send_flag_1 = 1;
			TYPE2_Canread_statust1= 0;
			break;
		}
		 default:{
			 if(data2 == 3)
			 {
					TYPE2_Canread_statust1 = 1;
					TYPE2_send_flag_1 = 1;
			 }
			 break;
		}
	}
	if(TYPE2_Canread_statust0 != 0 || TYPE2_Canread_statust1 != 0){//??????????????
		TYPE2_Can_read();
	}
		return 0;
}
unsigned char TYPE2_buf[24]={0};
int TYPE2_flag_can = 0;
int TYPE2_Can_read()
{
unsigned char wdata[8] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08};

	if(TYPE2_send_flag_1 == 1)
	{
		CanbusSend(CANBUS1,TYPE2_Can_Send_Id_1,(unsigned char *)wdata,8,CAN_STANDARD_MODE);
	}
	int queueLen;
	if(TYPE2_Canread_statust0 == 1 && TYPE2_send_flag_1 == 1)
	{			
		int ret = canbusReadIrq(CANBUS0,(int *)&TYPE2_TYPE2_Can_read_Id,TYPE2_Read_data0,&queueLen);
				if(ret == 0 && TYPE2_TYPE2_Can_read_Id != TYPE2_Can_Send_Id_0){
					unsigned char *pa;
					unsigned char tab[12];
					pa = (unsigned char *)&TYPE2_TYPE2_Can_read_Id;
					tab[0]=*((char *)pa+0);
					tab[1]=*((char *)pa+1);
					tab[2]=*((char *)pa+2);
					tab[3]=*((char *)pa+3);
					for(int i = 4;i < 12;i++){
						tab[i] = 	TYPE2_Read_data0[i-4];
					}
					for(int i = 0;i < 12;i++){
						TYPE2_buf[i] = tab[i];
						TYPE2_flag_can = 1;
					}
					TYPE2_Canread_statust0 = 0;
			}
		}
	if(TYPE2_send_flag_0 == 1)
	{
		CanbusSend(CANBUS0,TYPE2_Can_Send_Id_0,(unsigned char *)wdata,8,CAN_STANDARD_MODE);
	}
 if(TYPE2_Canread_statust1 == 1 && TYPE2_send_flag_0 == 1){
	 int ret1 = canbusReadIrq(CANBUS1,(int *)&TYPE2_TYPE2_Can_read_Id,TYPE2_Read_data1,&queueLen);
					if(ret1 ==0 && TYPE2_TYPE2_Can_read_Id!= TYPE2_Can_Send_Id_1){
						unsigned char *pa;
						unsigned char tab[12];
						pa = (unsigned char *)&TYPE2_TYPE2_Can_read_Id;
						tab[0]=*((char *)pa+0);
						tab[1]=*((char *)pa+1);
						tab[2]=*((char *)pa+2);
						tab[3]=*((char *)pa+3);
						for(int i = 4;i < 12;i++){
							tab[i] = 	TYPE2_Read_data1[i-4];
						}
						if(TYPE2_flag_can == 1){
								for(int i = 12;i < 24;i++){
								TYPE2_buf[i] = tab[i-12];
							}
							TYPE2_flag_can = 3;
						}
						else {
							for(int i = 0;i < 12;i++){
								TYPE2_buf[i] = tab[i];
							}
							TYPE2_flag_can = 2;
						}	
						TYPE2_Canread_statust1 = 0;
				}
			}
		if(TYPE2_Canread_statust0 == 1 && TYPE2_Canread_statust1 == 1 && TYPE2_send_flag_0 == 0 && TYPE2_send_flag_1 == 0){
			return 0;
		}
		else if(TYPE2_flag_can == 1 && TYPE2_send_flag_0==0) {//1 
			TYPE2_MCU_test_Send(2,12,TYPE2_buf);
			TYPE2_flag_can = 0;
			TYPE2_Canread_statust0 =0;
			TYPE2_send_flag_1 = 0;
		}
		else if(TYPE2_flag_can == 2 && TYPE2_send_flag_1 == 0){
			TYPE2_MCU_test_Send(2,12,TYPE2_buf);
			TYPE2_flag_can = 0;
			TYPE2_Canread_statust1 = 0;
			TYPE2_send_flag_0 = 1;
		}
		else if(TYPE2_flag_can == 3  && TYPE2_send_flag_0==1 && TYPE2_send_flag_1 == 1){
			TYPE2_MCU_test_Send(2,24,TYPE2_buf);
			TYPE2_flag_can = 0;
			TYPE2_Canread_statust0 = 0;
			TYPE2_Canread_statust1 = 0;
			TYPE2_send_flag_0 = 0;
			TYPE2_send_flag_1 = 1;
		}
		else if(TYPE2_flag_can == 1 && TYPE2_Canread_statust1 == 0){
			TYPE2_MCU_test_Send(2,12,TYPE2_buf);
			TYPE2_flag_can = 0;
			TYPE2_Canread_statust1 =0;
			TYPE2_send_flag_1 = 0;
		}
		else if(TYPE2_flag_can == 2 && TYPE2_Canread_statust0 == 0){
			TYPE2_MCU_test_Send(2,12,TYPE2_buf);
			TYPE2_flag_can = 0;
			TYPE2_Canread_statust1 =0;
			TYPE2_send_flag_0 = 0;
		}
}



int TYPE2_MCU_test_LTE()
{
	TYPE2_LTE_status	= mem909s_msmInit();
		wdgTps_process();
	
	if(TYPE2_LTE_status == 0){
		unsigned char stat[1]={0x02};
		TYPE2_MCU_test_Send(5,1,stat);
	}
	else if(TYPE2_LTE_status == -1){
		unsigned char stat1[1]={0x01};
		TYPE2_MCU_test_Send(5,1,stat1);
	}
	else {
		unsigned char stat0[1]={0x00};
		TYPE2_MCU_test_Send(5,1,stat0);
	}
}


int TYPE2_MCU_test_WDOG()
{
	if(TYPE2_Dog_flag == 0){
		wdgTps_init();
		TYPE2_Dog_flag = 1;
		wdgTps_process();
		unsigned char stat[1]={0x02};
		TYPE2_MCU_test_Send(3,1,stat);
	}
	else{
		unsigned char stat[1]={0x02};
		TYPE2_MCU_test_Send(3,1,stat);
	}

	return TYPE2_Dog_flag;
}

void TYPE2_MCU_test_Power_Management()
{
	if(TYPE2_Dog_flag == 1){
		wdgTps_process();
	}
}

//HEAD		ID			????	data	ROLLCNT  CRT  	 END4
//0X55 66	1BYTE		1BYTE		   			 0~255	          0X0D
// 0    1  2        3        4       5      6         7     

int TYPE2_MCU_test_Send(int id,int len_data,unsigned char *data)
{
	wdgTps_process();
		TYPE2_Read_ROLLCNT++;
		if(TYPE2_Read_ROLLCNT > TYPE2_Read_ROLLCNT_MAX){
			TYPE2_Read_ROLLCNT = 0;
		}
	int TYPE2_send_len;
	unsigned char Send_Crt;
	unsigned char crt;
	if(TYPE2_Dog_flag == 1)
	{
		wdgTps_process();
	}
	int i;
	unsigned char Send_data[200]={0};
	Send_data[0] = TYPE2_Send_Head_0;
	Send_data[1] = TYPE2_Send_Head_1;
	switch(id){
		case 1:{
			Send_data[2]=0x01;
			Send_data[3]=0x01;
			Send_data[4]=data[0];
			Send_data[5]=TYPE2_Send_ROLLCNT;
			Send_Crt = Send_data[0]^Send_data[1];
			for(i = 0;i < 4;i++)
			{
				Send_Crt = Send_Crt^Send_data[i+2];
			}
			Send_data[6]=Send_Crt;
			Send_data[7]=TYPE2_TAIL;
			TYPE2_send_len = 8;
			break;//665510xd
		} 
		case 2:{
					Send_data[2]=0x02;
					Send_data[3]=len_data;
					int i = 0;
					for(i = 0;i < len_data;i++)
					{
						Send_data[i+4] = data[i];
					}
					Send_data[len_data+4]=TYPE2_Send_ROLLCNT;
					Send_Crt = Send_data[0]^Send_data[1];
					for(i = 0;i < len_data+4;i++)
					{
						Send_Crt = Send_Crt^Send_data[i+2];
					}
					//strcat(Send_data,num);
					Send_data[len_data+5]=Send_Crt;
					Send_data[len_data+6]=TYPE2_TAIL;
					TYPE2_send_len = len_data+7;
					break;//66552cxxxxxxxxxxxxxd
				
			
			break;
		} 
		case 3:{
			Send_data[2]=0X03;
			Send_data[3]=0X01;
			int i;
			for(i = 0;i < len_data;i++)
				{
					Send_data[i+4] = data[i];
				}
			Send_data[5]=TYPE2_Send_ROLLCNT;
			Send_Crt = Send_data[0]^Send_data[1];
			for(i = 0;i < 4;i++)
			{
				Send_Crt = Send_Crt^Send_data[i+2];
			}
			Send_data[6]=Send_Crt;
			Send_data[7]=0x0d;
			TYPE2_send_len = 8;
			break;
		} 
		case 4:{
			Send_data[4]=0x04;
			break;
		} 
		case 5:{
			Send_data[2]=0X05;
			Send_data[3]=0X01;
			int i;
			for(i = 0;i < len_data;i++)
			{
				Send_data[i+4] = data[i];
			}
			Send_data[5]=TYPE2_Send_ROLLCNT;
			Send_Crt = Send_data[0]^Send_data[1];
			for(i = 0;i < 4;i++)
			{
				Send_Crt = Send_Crt^Send_data[i+2];
			}
			Send_data[6]=Send_Crt;
			Send_data[7]=0x0d;
			TYPE2_send_len = 8;
			break;
		}
	}
	TYPE2_Send_ROLLCNT++;
	if(TYPE2_Send_ROLLCNT >=TYPE2_Send_ROLLCNT_MAX)
	{
		TYPE2_Send_ROLLCNT = 0;
	}
int sret = uartSendIrq(UART1,Send_data,TYPE2_send_len,&TYPE2_rLenNow);
	if(sret == UART1_SEND_OVER){
		uartSendIrq(UART1,Send_data,TYPE2_send_len,&TYPE2_rLenNow);
	}
	return 0;
}

















