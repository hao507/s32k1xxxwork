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
unsigned char TYPE1_rData[60]={0}; 
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
int TYPE1_MCU_test_init()
{
	uartInit(UART0,9600);
	uartIrqInit(UART0);
	uartInit(UART1,9600);
	uartIrqInit(UART1);
	Canbus0Init(500000);
	Canbus0IrqInit();
	Canbus1Init(500000);
	Canbus1IrqInit();

	TYPE1_LTE_status = mem909sReadyGet();
	if(TYPE1_LTE_status == 1)
	{
		TYPE1_LTE_status = mem909s_msmInit();
	}
	wdgTps_init();
	TYPE1_Dog_flag = 1;
}

int TYPE1_MCU_test_LTE();
int TYPE1_MCU_test_Send(int id,int len_data,unsigned char *data);
int TYPE1_MCU_test_Canbus();
int TYPE1_MCU_test_WDOG();
void TYPE1_MCU_test_Power_Management();

int TYPE1_MCU_test_Analysis()
{	
	unsigned char crt = 0x00;
	int TYPE1_flag_1 = -1,flag_2 = 0,flag_3 = 0;
	unsigned char ID;
	unsigned char Read_Crt = 0x00;
	char wData[20] = "uart test\n\r";
	int t,j;
	for(int i = 0; i < 200;i++)
	{
		if(TYPE1_rData[i] == TYPE1_Read_Head_0 && TYPE1_rData[i+1] == TYPE1_Read_Head_1 && TYPE1_flag_1 == -1)
		{
			TYPE1_flag_1 = 0;
			TYPE1_ID_Start = i+2;
			if(TYPE1_rData[TYPE1_ID_Start+1] == 0X00){//data?0?
					crt = TYPE1_rData[TYPE1_ID_Start-2]^TYPE1_rData[TYPE1_ID_Start-1];
					for(j = 0;j < 3;j++)
						{
							crt = crt ^ TYPE1_rData[TYPE1_ID_Start+j];
						}
					if(TYPE1_rData[TYPE1_ID_Start+3] == crt)
						{
							TYPE1_flag_1 = 1;
						}
					else{
						return 0;
						}//?????
					if(TYPE1_rData[TYPE1_ID_Start+2] == TYPE1_Read_ROLLCNT && TYPE1_flag_1 == 1)
						{
							flag_2 = 1;
							TYPE1_flag_1 = 2;
						}
					else{
							flag_2 = 1;
							TYPE1_flag_1 = 2;
						}//????
				}
				if(TYPE1_rData[TYPE1_ID_Start+1] == 0X02) //len = 2
				{
						crt = TYPE1_rData[TYPE1_ID_Start-2]^TYPE1_rData[TYPE1_ID_Start-1];
						for(j = 0;j < 5;j++)
						{
							crt = crt ^ TYPE1_rData[TYPE1_ID_Start+j];
						}
						if(TYPE1_rData[TYPE1_ID_Start+5] == crt)
						{
							TYPE1_flag_1 = 1;
						}
					else
						{
							TYPE1_flag_1 = 1;
						}//?????
					if(TYPE1_rData[TYPE1_ID_Start+4] == TYPE1_Read_ROLLCNT && TYPE1_flag_1 == 1)
						{
								flag_2 = 1;
						}
					else
						{
							flag_2 = 1;
						}//????
					}
			}
			if(flag_2 == 1 && TYPE1_rData[i] == TYPE1_TAIL)
			{
					flag_2 = 2,flag_3 = 1;
					 break;
			}
		}
	if(flag_2 == 2 && flag_3 == 1){
		ID = TYPE1_rData[TYPE1_ID_Start];
		switch(ID){
			case 1:{
				//uartSendIrq(UART1,(unsigned char *)wData,strlen(wData),&TYPE1_rLenNow);
				TYPE1_MCU_test_Send(1,1,TYPE1_uartsend_ok);
				break;                
			}
			case 2:{
				TYPE1_MCU_test_Canbus();
				break;
			}
			case 3:{
				int i =TYPE1_MCU_test_WDOG();
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
	if(TYPE1_Dog_flag == 1)
	{
		wdgTps_process();
	}

	return 0;
}

int TYPE1_MCU_test_Canbus()
{
	
	
		wdgTps_process();
	
	unsigned char data1 = TYPE1_rData[TYPE1_ID_Start+2];
	unsigned char data2 = TYPE1_rData[TYPE1_ID_Start+3];
	int Can_Send_Id = 0x123;
	unsigned char wdata[] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08};
	unsigned char TYPE1_buf[24]={0};
	int queueLen,tiomout = 10000;
	int flag_0 = 0;
	int TYPE1_flag_1 = 0;

	switch(data1){
		case 0:{
			break;
		} 
		case 2:{
				TYPE1_Canread_statust0 = 1;
				TYPE1_Can_read();
				break;
		}
		case 1:{
			Canbus0Send(Can_Send_Id,(unsigned char *)wdata,8,CAN_STANDARD_MODE);
			flag_0 = 1;
			break;
		}
		 default:{
			 if(data1 == 3)
			 {
				 TYPE1_Canread_statust0 = 1;
				//	TYPE1_Can_read();
				 Canbus0Send(Can_Send_Id,(unsigned char *)wdata,8,CAN_STANDARD_MODE);
				 flag_0 = 2;
			 }
				//canbus0ReadIrq((int *)&TYPE1_TYPE1_Can_read_Id,TYPE1_Read_data0,&queueLen);
			 break;
		}
	}
	
		switch(data2){
		case 0:{
			break;
		} 
		case 2:{
			TYPE1_Canread_statust1 = 1;
			TYPE1_Can_read();
			break;
		}
		case 1:{
			Canbus1Send(Can_Send_Id,(unsigned char *)wdata,8,CAN_STANDARD_MODE);
			TYPE1_flag_1 = 1;
			break;
		}
		 default:{
			 if(data2 == 3)
			 {
					TYPE1_Canread_statust1 = 1;
					TYPE1_Can_read();
					Canbus0Send(Can_Send_Id,(unsigned char *)wdata,8,CAN_STANDARD_MODE);
					TYPE1_flag_1 = 2;
			 }

			 TYPE1_Canread_statust1 = 1;
			 break;
		}
	}
	if(flag_0 == 1 && TYPE1_flag_1 != 1)
		TYPE1_MCU_test_Send(2,12,TYPE1_send_TYPE1_buf);
	else if(TYPE1_flag_1 == 1 && flag_0 != 1 )
		TYPE1_MCU_test_Send(2,12,TYPE1_send_TYPE1_buf);
	else if(flag_0 == 1 && TYPE1_flag_1 == 1)//?????
		TYPE1_MCU_test_Send(2,24,TYPE1_send_TYPE1_buf);
	if(TYPE1_Canread_statust0 != 0 || TYPE1_Canread_statust1 != 0){//??????????????
		TYPE1_Can_read();
	}
		return 0;
}
unsigned char TYPE1_buf[24]={0};
int TYPE1_Can_read()
{
	int flag_can = 0;
	int queueLen;
	if(TYPE1_Canread_statust0 == 1)
	{			
		canbus0ReadIrq((int *)&TYPE1_TYPE1_Can_read_Id,TYPE1_Read_data0,&queueLen);
				if(strlen((char *)TYPE1_Read_data0) > 0 ){
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
						flag_can = 1;
					}
					TYPE1_Canread_statust0 = 0;
			}
		}
 if(TYPE1_Canread_statust1 == 1){
	 canbus1ReadIrq((int *)&TYPE1_TYPE1_Can_read_Id,TYPE1_Read_data1,&queueLen);
					if(strlen((char *)TYPE1_Read_data1) > 0 ){
						unsigned char *pa;
						unsigned char tab[12];
						pa = (unsigned char *)&TYPE1_TYPE1_Can_read_Id;
						tab[0]=*((char *)pa+0);
						tab[1]=*((char *)pa+1);
						tab[2]=*((char *)pa+2);
						tab[3]=*((char *)pa+3);
						for(int i = 4;i < 12;i++){
							tab[i] = 	TYPE1_Read_data1[i-4];
						}
						if(flag_can == 1){
								for(int i = 12;i < 24;i++){
								TYPE1_buf[i] = tab[i-12];
							}
							flag_can = 3;
						}
						else {
							for(int i = 0;i < 12;i++){
								TYPE1_buf[i] = tab[i];
							}
							flag_can = 2;
						}	
						TYPE1_Canread_statust1 = 0;
				}
			}
		if(flag_can == 1 && 0 == TYPE1_Canread_statust1)  {
			TYPE1_MCU_test_Send(2,12,TYPE1_buf);
			flag_can = 0;
		}
		else if(flag_can == 2 && 0 == TYPE1_Canread_statust0){
			TYPE1_MCU_test_Send(2,12,TYPE1_buf);
			flag_can = 0;
		}
		else if(flag_can == 3){
			TYPE1_MCU_test_Send(2,24,TYPE1_buf);
			flag_can = 0;
		}
}

int TYPE1_LTE_init_flag = 0;

int TYPE1_MCU_test_LTE()
{

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



int TYPE1_flag_1 = 0;
void TYPE1_MCU_Automatic_testing_Start()
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
	uartReadIrq(UART1,TYPE1_rData,sizeof(buf),&TYPE1_rLenNow);
		for(int i= 0;i < TYPE1_rLenNow;i++)
		{
			if(TYPE1_rData[i] == TYPE1_TAIL)
			{
				TYPE1_rLenNow = 0;
				TYPE1_MCU_test_Analysis();
				uartReadIrqReset(UART1);
				memset(TYPE1_rData,0,sizeof(TYPE1_rData));
				flag_f = 1;
				break;
			}
		}
		
	if(TYPE1_Canread_statust0 != 0 || TYPE1_Canread_statust1 != 0){
		TYPE1_Can_read();
	}
}












