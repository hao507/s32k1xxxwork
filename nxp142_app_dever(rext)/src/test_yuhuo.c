#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "uart.h"
#include "usertype.h"
#include "canbus.h"
#include "wdgTps.h"


#define endOkFind 1
#define endErrorFind 2
#define none 5
#define sendFail 3
#define beginFind 35


#define Read_Head_0 0x55
#define Read_Head_1 0x66
#define Send_Head_0 0x66
#define Send_Head_1 0x55
#define TAIL 0x0d


int rLenNow;
unsigned char Send_ROLLCNT = 0x00;
unsigned char Read_ROLLCNT = 0x00;
unsigned char Send_ROLLCNT_MAX = 0xFF;
unsigned char Read_ROLLCNT_MAX = 0XFF;
unsigned char rData[200]={0}; 
int Dog_flag = 0;
int ID_Start;



int type0_MCU_test_init()
{
	uartInit(UART0,9600);
	uartIrqInit(UART0);
	uartInit(UART1,115200);
	uartIrqInit(UART1);
	Canbus0Init(500000);
	Canbus0IrqInit();
}

int type0_MCU_test_LTE();
int type0_MCU_test_Send(int id,int len_data,unsigned char *data);
int type0_MCU_test_Canbus();
int type0_MCU_test_WDOG();
void type0_MCU_test_Power_Management();

int type0_MCU_test_Analysis()
{	
	int flag_1 = 0,flag_2 = 0,flag_3 = 0;
	unsigned char ID;
	char wData[] = "uart test\n\r";
	int t;
	for(int i = 0;i < 12;i++){
		if(rData[i] == Read_Head_0 && rData[i+1] == Read_Head_1){
			flag_1 = 1;
			ID_Start = i+2;
		}
		 if(flag_1 == 1){
			if(rData[ID_Start+1] == 0X00){//data为0时
				if(rData[ID_Start+2] == Read_ROLLCNT)
				{
					Read_ROLLCNT++;
					if(Read_ROLLCNT > Read_ROLLCNT_MAX){
						Read_ROLLCNT = 0;
					}
					flag_2 = 1;
				}
				else{
				}//数据丢包
			}
			else {//data为1时
				if(rData[ID_Start+3] == Read_ROLLCNT){
					Read_ROLLCNT++;
					if(Read_ROLLCNT > Read_ROLLCNT_MAX){
						Read_ROLLCNT = 0;
					}
						flag_2 = 1;
				}
				else{
					
				}//数据丢包
			}
		}
		 if(flag_2 == 1 && rData[i] == TAIL){
			flag_3 = 1;
			 break;
		}
	}
	if(flag_2 == 1 && flag_3 == 1){
		ID = rData[ID_Start];
		switch(ID){
			case 1:{
				uartSendIrq(UART0,(unsigned char *)wData,strlen(wData),&rLenNow);
				type0_MCU_test_Send(1,0,NULL);
				break;
			}
			case 2:{
				type0_MCU_test_Canbus();
				break;
			}
			case 3:{
				int i = type0_MCU_test_WDOG();
				break;
			}
			case 4:{
			 type0_MCU_test_Power_Management();
				break;
			}
			case 5:{
				type0_MCU_test_LTE();
				break;
			}
		}
	}
	if(Dog_flag == 1)
	{
		wdgTps_process();
	}
	return 0;
}



int type0_MCU_test_Canbus()
{
	if(Dog_flag == 1)
	{
		wdgTps_process();
	}
	unsigned char data = rData[4];
	int Can_Send_Id = 0x123;
	int Can_Read_Id;
	unsigned char wdata[] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08};
	
	char buf[200]={0};
	int queueLen;
	unsigned char Read_data[200]={0};
	switch(data){
		case 0:{
			type0_MCU_test_Send(2,0,NULL);
			break;
		} 
		case 1:{
			Canbus0Send(Can_Send_Id,(unsigned char *)wdata,strlen((char *)wdata),CAN_STANDARD_MODE);
			type0_MCU_test_Send(2,0,NULL);
			break;
		}
		case 2:{
			canbus0ReadIrq((int *)&Can_Read_Id,Read_data,&queueLen);
			unsigned char *pa;
			unsigned char tab[12];
			pa = (unsigned char *)&Can_Read_Id;
			tab[0]=*((char *)pa+0);
			tab[1]=*((char *)pa+1);
			tab[2]=*((char *)pa+2);
			tab[3]=*((char *)pa+3);
			for(int i = 4;i < 12;i++){
				tab[i] = 	Read_data[i-4];
			}
			type0_MCU_test_Send(2,12,tab);
			break;
		}
		case 3:{
			canbus0ReadIrq(&Can_Read_Id,Read_data,&queueLen);
			Canbus0Send(Can_Send_Id,(unsigned char *)wdata,strlen((char*)wdata),CAN_STANDARD_MODE);
			unsigned char *pa;
			unsigned char tab[12];
			pa = (unsigned char *)&Can_Read_Id;
			tab[0]=*((char *)pa+0);
			tab[1]=*((char *)pa+1);
			tab[2]=*((char *)pa+2);
			tab[3]=*((char *)pa+3);
			for(int i = 4;i < 12;i++){
				tab[i] = 	Read_data[i-4];
			}
			type0_MCU_test_Send(2,12,tab);
			break;
		}
	}

}





int type0_MCU_test_LTE()
{
	if(Dog_flag == 1){
		wdgTps_process();
	}
	char *AT_Order = "AT\n\r";
	char mate_endOk[2]="OK";	
	char mate_endError[5]="ERROR";
	char Lte_rData[200]={0};
	int rLenNow = 0;
	int timeout = 1000;
	int sendStatus;
	sendStatus = sendFail;
	for(int i = 0 ;i < timeout;i++)
	{
		uartSendBlock(UART1,(unsigned char *)AT_Order,strlen(AT_Order),timeout);
		uartReadIrq(UART1,(unsigned char *)Lte_rData,sizeof(Lte_rData),&rLenNow);
		for(int i = 0;i < 5000;i++)
		{
			if(strlen(Lte_rData)> 0)
			{
					for(int i=0; i<rLenNow; i++){
						if(strncmp(Lte_rData+i,AT_Order,strlen(AT_Order)) == 0){
							sendStatus = beginFind;
						}
						else if(strncmp(mate_endOk,Lte_rData+i,strlen(mate_endOk)) == 0 && (sendStatus==beginFind)){
							sendStatus = endOkFind;
							uartReadIrqReset(UART1);
							unsigned char stat[1]={0x02};
							type0_MCU_test_Send(5,1,stat);
							return sendStatus;
						}
						else if(strncmp(mate_endError,Lte_rData+i,strlen(mate_endError)) == 0 && (sendStatus==beginFind)){
							sendStatus = endErrorFind;
							uartReadIrqReset(UART1);
							unsigned char stat[1]={0x01};
							type0_MCU_test_Send(5,1,stat);
							return sendStatus;
					}
				}
			}
			if(i%1000 == 0 && Dog_flag == 1){
					wdgTps_process();
			}
		}
	}
	unsigned char stat[1]={0x00};
	type0_MCU_test_Send(5,1,stat);
	return none;
}


int type0_MCU_test_WDOG()
{
	if(Dog_flag == 0){
		wdgTps_init();
		Dog_flag = 1;
		wdgTps_process();
		unsigned char stat[1]={0x02};
		type0_MCU_test_Send(3,1,stat);
	}
	else{
		unsigned char stat[1]={0x02};
		type0_MCU_test_Send(3,1,stat);
	}

	return Dog_flag;
}

void type0_MCU_test_Power_Management()
{
	if(Dog_flag == 1){
		wdgTps_process();
	}
}

unsigned char buf[]={0x55,0x66,0x05,0x00,0x00,0x0d};
void type0_MCU_Automatic_testing_Start()
{
	if(Dog_flag == 1){
		wdgTps_process();
	}
	int timeout = 10000,i;
	
//	buf[4] = buf[4]+1;
//	for(int i = 0;i < 6;i++){
//		rData[i] = buf[i];
//		}	
	type0_MCU_test_Analysis();
	uartReadIrq(UART0,rData,sizeof(rData),&rLenNow);
	for(i = 0;i < timeout;i++)
	{
		if(rLenNow > 6)
		{
				rLenNow = 0;
				type0_MCU_test_Analysis();
				uartReadIrqReset(UART0);
				memset(rData,0,sizeof(rData));
		}
	}
}




int type0_MCU_test_Send(int id,int len_data,unsigned char *data)
{
	if(Dog_flag == 1)
	{
		wdgTps_process();
	}

	unsigned char Send_data[200]={0};
	Send_data[0] = Send_Head_0;
	Send_data[1] = Send_Head_1;
	switch(id){
		case 1:{
			Send_data[2]=0x01;
			Send_data[3]=0x00;
			Send_data[4]=Send_ROLLCNT;
			Send_data[5]=TAIL;
			break;//665510xd
		} 
		case 2:{
			Send_data[2]=0x02;
			switch(len_data){
				case 0:{
					Send_data[3]=0x00;
					Send_data[4]=Send_ROLLCNT;
					Send_data[5]=TAIL;
					break;//665520xd
				}
				case 12:{
					Send_data[3]=0x0C;
					int i = 0;
					for(i = 0;i < len_data;i++)
					{
						Send_data[i+4] = data[i];
					}
					//strcat(Send_data,num);
					Send_data[len_data+4]=Send_ROLLCNT;
					Send_data[len_data+5]=TAIL;
					break;//66552cxxxxxxxxxxxxxd
				}
			}
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
			Send_data[5]=Send_ROLLCNT;
			Send_data[6]=0x0d;
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
			Send_data[5]=Send_ROLLCNT;
			Send_data[6]=0x0d;
			break;
		} 
	}
	Send_ROLLCNT++;
	if(Send_ROLLCNT > Send_ROLLCNT_MAX){
		Send_ROLLCNT = 0;
	}
	uartSendIrq(UART0,(unsigned char *)Send_data,strlen((char *)Send_data),&rLenNow);
	return 0;
}







