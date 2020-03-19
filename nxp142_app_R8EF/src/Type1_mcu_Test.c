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

#define TYPE1_endOkFind 1
#define TYPE1_endErrorFind 2
#define TYPE1_none 5
#define TYPE1_sendFail 3
#define TYPE1_beginFind 35


#define TYPE1_Read_Head_0 0x55
#define TYPE1_Read_Head_1 0x66
#define TYPE1_Send_Head_0 0x66
#define TYPE1_Send_Head_1 0x55
#define TYPE1_TAIL 0x0d


int TYPE1_rLenNow;
unsigned char TYPE1_Send_ROLLCNT = 0x00;
unsigned char TYPE1_Read_ROLLCNT = 0x00;
unsigned char TYPE1_Send_ROLLCNT_MAX = 0xFF;
unsigned char TYPE1_Read_ROLLCNT_MAX = 0XFF;
unsigned char TYPE1_rData[100]={0}; 
int TYPE1_Dog_flag = 0;
int TYPE1_ID_Start;

unsigned char TYPE1_uartsend_error[1]={0x01};
unsigned char TYPE1_uartsend_ok[1]={0x02};
int TYPE1_send_len = 0;
int TYPE1_LTE_status = -1;
int TYPE1_Can_read();
int TYPE1_Canread_statust0 = 0;
int TYPE1_Canread_statust1 = 0;
int TYPE1_TYPE1_Can_read_Id;
unsigned char TYPE1_Read_data0[200]={0};
unsigned char TYPE1_Read_data1[200]={0};
unsigned char TYPE1_send_TYPE1_buf[24]={0};
int TYPE1_LTE_init_flag = 0;
int TYPE1_MCU_test_init()
{
	wdgTps_init();
	uartInit(UART0,9600);
	uartIrqInit(UART0);
	uartInit(UART1,9600);
	uartIrqInit(UART1);
	Canbus0Init(500000);
	Canbus0IrqInit();
	Canbus1Init(500000);
	Canbus1IrqInit();
	TYPE1_Dog_flag = 1;
}
int TYPE1_flag_1 = 0;
int TYPE1_MCU_test_LTE();
int TYPE1_MCU_test_Send(int id,int len_data,unsigned char *data);
int TYPE1_MCU_test_Canbus();
int TYPE1_MCU_test_WDOG();
void TYPE1_MCU_test_Power_Management();
int TYPE1_MCU_test_Analysis(int rdatalen);

void TYPE1_MCU_Automatic_testing_Start(char *rData,int rLenNow)
{

	if(TYPE1_flag_1 == 0)
	{
		TYPE1_Canread_statust0 = 1;
		TYPE1_flag_1=1;
	}
		wdgTps_process();
	if(TYPE1_Canread_statust0 != 0 || TYPE1_Canread_statust1 != 0){
		TYPE1_Can_read();
	}
	int timeout = 10000,i;
	int flag_f = 0;
	unsigned char buf [40] ={0};
		TYPE1_rLenNow = rLenNow;
		for(int i = 0;i < TYPE1_rLenNow;i++){
			 TYPE1_rData[i]=rData[i];
		}
		for(int i= 0;i < TYPE1_rLenNow;i++)
		{
			if(TYPE1_rData[i] == TYPE1_Read_Head_0 && TYPE1_rData[i+1] == TYPE1_Read_Head_1){
				flag_f = 1;
			}
			if(TYPE1_rData[i] == TYPE1_TAIL && flag_f == 1)
			{
				TYPE1_MCU_test_Analysis(TYPE1_rLenNow);
				uartReadIrqReset(UART1);
				memset(TYPE1_rData,0,sizeof(TYPE1_rData));
				memset(rData,0,rLenNow);
				break;
			}
		}
		memset(TYPE1_rData,0,sizeof(TYPE1_rData));
	if(TYPE1_Canread_statust0 != 0 || TYPE1_Canread_statust1 != 0){
		TYPE1_Can_read();
	}
}
int TYPE1_MCU_test_Analysis(rdatalen)
{	
	wdgTps_process();
	unsigned char crt = 0x00;
	unsigned char Read_Crt = 0x00;
	char data[20]={0};
	int read_data_len = 0;
	int data_strat=0;
	int readover_falg = 0;
	for(int i = 0;i < rdatalen;i++){
		if(TYPE1_rData[i] == TYPE1_Read_Head_0){
			
			data_strat = i;
			if(TYPE1_rData[data_strat+3] == 0){
				read_data_len = 7;
				break;
			}
			else if(TYPE1_rData[data_strat+3] == 2){
				read_data_len = 9;
				break;
			}
		}
	}
	crt = TYPE1_rData[data_strat] ^ TYPE1_rData[data_strat+1];
	for(int i = data_strat+2;i < data_strat+read_data_len-2;i++){
		crt = crt ^ TYPE1_rData[i];
	}
	if(crt == TYPE1_rData[data_strat+read_data_len-2]){
		TYPE1_ID_Start = data_strat+2;
		char ID = TYPE1_rData[data_strat+2];
			switch(ID){
			case 1:{
				TYPE1_MCU_test_Send(1,1,TYPE1_uartsend_ok);
				break;                
			}
			case 2:{
				TYPE1_MCU_test_Canbus();
				break;
			}
			case 3:{
				TYPE1_MCU_test_WDOG();
				break;
			}
			case 4:{
			TYPE1_MCU_test_Power_Management();
				break;
			}
			case 5:{
				TYPE1_MCU_test_LTE();
				break;
			}
		}
	}
	else{
		return -1;
	}
}
int type1_Can_Send_Id_0 = 0x123;
int type1_Can_Send_Id_1 = 0x234;
int type1_send_flag_0 = 0;
int type1_send_flag_1 = 0;

int TYPE1_MCU_test_Canbus()
{
		wdgTps_process();
	unsigned char data1 = TYPE1_rData[TYPE1_ID_Start+2];
	unsigned char data2 = TYPE1_rData[TYPE1_ID_Start+3];
	unsigned char TYPE1_buf[24]={0};
	int queueLen,tiomout = 10000;
	int flag_0 = 0;
	int TYPE1_flag_1 = 0;

	switch(data1){
		case 0:{
			TYPE1_Canread_statust0 = 0;
			type1_send_flag_0 = 0;
			break;
		} 
		case 1:{
				TYPE1_Canread_statust0 = 1;
				type1_send_flag_0 = 0;
				break;
		}
		case 2:{
			type1_send_flag_0 = 1;
			TYPE1_Canread_statust0 = 0;
			break;
		}
		 default:{
			 if(data1 == 3)
			 {
				 TYPE1_Canread_statust0 = 1;
				 type1_send_flag_0 =1;
			 }
			 break;
		}
	}
	
		switch(data2){
		case 0:{
			type1_send_flag_1 = 0;
			TYPE1_Canread_statust1 = 0;
			break;
		} 
		case 1:{
			TYPE1_Canread_statust1 = 1;
			type1_send_flag_1 = 0;
			break;
		}
		case 2:{
			type1_send_flag_1 = 1;
			TYPE1_Canread_statust1= 0;
			break;
		}
		 default:{
			 if(data2 == 3)
			 {
					TYPE1_Canread_statust1 = 1;
					type1_send_flag_1 = 1;
			 }
			 break;
		}
	}
	if(TYPE1_Canread_statust0 != 0 || TYPE1_Canread_statust1 != 0){//??????????????
		TYPE1_Can_read();
	}
		return 0;
}
unsigned char TYPE1_buf[24]={0};
int type1_flag_can = 0;
int TYPE1_Can_read()
{
unsigned char wdata[8] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08};

	if(type1_send_flag_0 == 1)
	{
		Canbus0Send(type1_Can_Send_Id_1,(unsigned char *)wdata,8,CAN_STANDARD_MODE);
		type1_send_flag_0 = 0;
	}
	int queueLen;
	if(TYPE1_Canread_statust0 == 1)
	{			
		int ret = canbus0ReadIrq((int *)&TYPE1_TYPE1_Can_read_Id,TYPE1_Read_data0,&queueLen);
				if(ret == 0 && TYPE1_TYPE1_Can_read_Id!= type1_Can_Send_Id_1){
					unsigned char *pa;
					unsigned char tab[12];
					pa = (unsigned char *)&TYPE1_TYPE1_Can_read_Id;
					tab[0]=*((char *)pa+0);
					tab[1]=*((char *)pa+1);
					tab[2]=*((char *)pa+2);
					tab[3]=*((char *)pa+3);
					for(int i = 4;i < 12;i++){
						tab[i] = 	TYPE1_Read_data0[i-4];
					}
					for(int i = 0;i < 12;i++){
						TYPE1_buf[i] = tab[i];
						type1_flag_can = 1;
					}
					TYPE1_Canread_statust0 = 0;
			}
		}
		 if(type1_flag_can == 1) {
			 TYPE1_TYPE1_Can_read_Id = 0x00;
			TYPE1_MCU_test_Send(2,12,TYPE1_buf);
			type1_flag_can = 0;
			TYPE1_Canread_statust0 =0;
			type1_send_flag_0 = 0;
			memset(TYPE1_Read_data0,0,200);
		}
}



int TYPE1_MCU_test_LTE()
{
		TYPE1_LTE_status = mem909s_Init();
		wdgTps_process();
	
	if(TYPE1_LTE_status == 0){
		unsigned char stat[1]={0x02};
		TYPE1_MCU_test_Send(5,1,stat);
	}
	else if(TYPE1_LTE_status == -1){
		unsigned char stat1[1]={0x01};
		TYPE1_MCU_test_Send(5,1,stat1);
	}
	else {
		unsigned char stat0[1]={0x00};
		TYPE1_MCU_test_Send(5,1,stat0);
	}
}


int TYPE1_MCU_test_WDOG()
{
	if(TYPE1_Dog_flag == 0){
		wdgTps_init();
		TYPE1_Dog_flag = 1;
		wdgTps_process();
		unsigned char stat[1]={0x02};
		TYPE1_MCU_test_Send(3,1,stat);
	}
	else{
		unsigned char stat[1]={0x02};
		TYPE1_MCU_test_Send(3,1,stat);
	}

	return TYPE1_Dog_flag;
}

void TYPE1_MCU_test_Power_Management()
{
	if(TYPE1_Dog_flag == 1){
		wdgTps_process();
	}
}

//HEAD		ID			????	data	ROLLCNT  CRT  	 END4
//0X55 66	1BYTE		1BYTE		   			 0~255	          0X0D
// 0    1  2        3        4       5      6         7     

int TYPE1_MCU_test_Send(int id,int len_data,unsigned char *data)
{
	wdgTps_process();
		TYPE1_Read_ROLLCNT++;
		if(TYPE1_Read_ROLLCNT > TYPE1_Read_ROLLCNT_MAX){
			TYPE1_Read_ROLLCNT = 0;
		}
	int TYPE1_send_len;
	unsigned char Send_Crt;
	unsigned char crt;
	if(TYPE1_Dog_flag == 1)
	{
		wdgTps_process();
	}
	int i;
	unsigned char Send_data[200]={0};
	Send_data[0] = TYPE1_Send_Head_0;
	Send_data[1] = TYPE1_Send_Head_1;
	switch(id){
		case 1:{
			Send_data[2]=0x01;
			Send_data[3]=0x01;
			Send_data[4]=data[0];
			Send_data[5]=TYPE1_Send_ROLLCNT;
			Send_Crt = Send_data[0]^Send_data[1];
			for(i = 0;i < 4;i++)
			{
				Send_Crt = Send_Crt^Send_data[i+2];
			}
			Send_data[6]=Send_Crt;
			Send_data[7]=TYPE1_TAIL;
			TYPE1_send_len = 8;
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
					Send_data[len_data+4]=TYPE1_Send_ROLLCNT;
					Send_Crt = Send_data[0]^Send_data[1];
					for(i = 0;i < len_data+4;i++)
					{
						Send_Crt = Send_Crt^Send_data[i+2];
					}
					//strcat(Send_data,num);
					Send_data[len_data+5]=Send_Crt;
					Send_data[len_data+6]=TYPE1_TAIL;
					TYPE1_send_len = len_data+7;
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
			Send_data[5]=TYPE1_Send_ROLLCNT;
			Send_Crt = Send_data[0]^Send_data[1];
			for(i = 0;i < 4;i++)
			{
				Send_Crt = Send_Crt^Send_data[i+2];
			}
			Send_data[6]=Send_Crt;
			Send_data[7]=0x0d;
			TYPE1_send_len = 8;
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
			Send_data[5]=TYPE1_Send_ROLLCNT;
			Send_Crt = Send_data[0]^Send_data[1];
			for(i = 0;i < 4;i++)
			{
				Send_Crt = Send_Crt^Send_data[i+2];
			}
			Send_data[6]=Send_Crt;
			Send_data[7]=0x0d;
			TYPE1_send_len = 8;
			break;
		}
	}
	TYPE1_Send_ROLLCNT++;
	if(TYPE1_Send_ROLLCNT >=TYPE1_Send_ROLLCNT_MAX)
	{
		TYPE1_Send_ROLLCNT = 0;
	}
int sret = uartSendIrq(UART1,Send_data,TYPE1_send_len,&TYPE1_rLenNow);
	if(sret == UART1_SEND_OVER){
		uartSendIrq(UART1,Send_data,TYPE1_send_len,&TYPE1_rLenNow);
	}
	return 0;
}

















