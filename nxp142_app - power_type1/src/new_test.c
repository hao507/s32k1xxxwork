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
unsigned char rData[60]={0}; 
int Dog_flag = 0;
int ID_Start;

unsigned char uartsend_error[1]={0x01};
unsigned char uartsend_ok[1]={0x02};
int send_len = 0;
int LTE_status = -1;

int type0_MCU_test_init()
{
	uartInit(UART0,9600);
	uartIrqInit(UART0);
	uartInit(UART1,115200);
	uartIrqInit(UART1);
	Canbus0Init(500000);
	Canbus0IrqInit();
	Canbus1Init(500000);
	Canbus1IrqInit();
		Dog_flag = 1;
}
int type0_MCU_test_LTE();
int type0_MCU_test_Send(int id,int len_data,unsigned char *data);
int type0_MCU_test_Canbus();
int type0_MCU_test_WDOG();
void type0_MCU_test_Power_Management();
int Can_read();
int Canread_statust0 = 0;
int Canread_statust1 = 0;
int Can_Read_Id;
unsigned char Read_data0[200]={0};
unsigned char Read_data1[200]={0};
unsigned char send_buf[24]={0};

int int MCU_test_Analysis(int rdatalen)
{
	wdgTps_process();
	unsigned char crt = 0x00;
	unsigned char Read_Crt = 0x00;
	char data[20]={0};
	int read_data_len = 0;
	int data_strat=0;
	for(int i = 0;i < rdatalen;i++){
		if(rData[i] == Read_Head_0){
			data_strat = i;
			if(rData[data_strat+3] == 0){
				read_data_len = 7;
				break;
			}
			else if(rData[data_strat+3] == 2){
				read_data_len = 9;
				break;
			}
		}
	}
	crt = rData[data_strat] ^ rData[data_strat+1];
	for(int i = 0;i < read_data_len-2;i++){
		crt = crt ^ rData[i];
	}
	if(crt == rData[data_strat+read_data_len-2]){
		char ID = rData[data_strat+2];
			switch(ID){
			case 1:{
				type0_MCU_test_Send(1,1,uartsend_ok);
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
	else{
		return -1;
	}
}

int type0_MCU_test_Canbus()
{
		wdgTps_process();
	
	unsigned char data1 = rData[ID_Start+2];
	unsigned char data2 = rData[ID_Start+3];
	int Can_Send_Id = 0x123;
	unsigned char wdata[] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08};
	unsigned char buf[24]={0};
	int queueLen,tiomout = 10000;
	int flag_0 = 0;
	int flag_1 = 0;

	switch(data1){
		case 0:{
			break;
		} 
		case 2:{
				Canread_statust0 = 1;
				Can_read();
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
				 Canread_statust0 = 1;
				 Canbus0Send(Can_Send_Id,(unsigned char *)wdata,8,CAN_STANDARD_MODE);
				 flag_0 = 2;
			 }
			 break;
		}
	}
	
		switch(data2){
		case 0:{
			break;
		} 
		case 2:{
			Canread_statust1 = 1;
			Can_read();
			break;
		}
		case 1:{
			Canbus1Send(Can_Send_Id,(unsigned char *)wdata,8,CAN_STANDARD_MODE);
			flag_1 = 1;
			break;
		}
		 default:{
			 if(data2 == 3)
			 {
					Canread_statust1 = 1;
					Can_read();
					Canbus0Send(Can_Send_Id,(unsigned char *)wdata,8,CAN_STANDARD_MODE);
					flag_1 = 2;
			 }
			 Canread_statust1 = 1;
			 break;
		}
	}
	if(flag_0 == 1 && flag_1 != 1)
		type0_MCU_test_Send(2,12,send_buf);
	else if(flag_1 == 1 && flag_0 != 1 )
		type0_MCU_test_Send(2,12,send_buf);
	else if(flag_0 == 1 && flag_1 == 1)
		type0_MCU_test_Send(2,24,send_buf);
	if(Canread_statust0 != 0 || Canread_statust1 != 0){
		Can_read();
	}
		return 0;
}

	unsigned char buf[24]={0};
int Can_read()
{
	int flag_can = 0;
	int queueLen;
	if(Canread_statust0 == 1)
	{			
		canbus0ReadIrq((int *)&Can_Read_Id,Read_data0,&queueLen);
				if(strlen((char *)Read_data0) > 0 ){
					unsigned char *pa;
					unsigned char tab[12];
					pa = (unsigned char *)&Can_Read_Id;
					tab[0]=*((char *)pa+0);
					tab[1]=*((char *)pa+1);
					tab[2]=*((char *)pa+2);
					tab[3]=*((char *)pa+3);
					for(int i = 4;i < 12;i++){
						tab[i] = 	Read_data0[i-4];
					}
					for(int i = 0;i < 12;i++){
						buf[i] = tab[i];
						flag_can = 1;
					}
					Canread_statust0 = 0;
			}
		}
 if(Canread_statust1 == 1){
	 canbus1ReadIrq((int *)&Can_Read_Id,Read_data1,&queueLen);
					if(strlen((char *)Read_data1) > 0 ){
						unsigned char *pa;
						unsigned char tab[12];
						pa = (unsigned char *)&Can_Read_Id;
						tab[0]=*((char *)pa+0);
						tab[1]=*((char *)pa+1);
						tab[2]=*((char *)pa+2);
						tab[3]=*((char *)pa+3);
						for(int i = 4;i < 12;i++){
							tab[i] = 	Read_data1[i-4];
						}
						if(flag_can == 1){
								for(int i = 12;i < 24;i++){
								buf[i] = tab[i-12];
							}
							flag_can = 3;
						}
						else {
							for(int i = 0;i < 12;i++){
								buf[i] = tab[i];
							}
							flag_can = 2;
						}	
						Canread_statust1 = 0;
				}
			}
		if(flag_can == 1 && 0 == Canread_statust1)  {
			type0_MCU_test_Send(2,12,buf);
			flag_can = 0;
		}
		else if(flag_can == 2 && 0 == Canread_statust0){
			type0_MCU_test_Send(2,12,buf);
			flag_can = 0;
		}
		else if(flag_can == 3){
			type0_MCU_test_Send(2,24,buf);
			flag_can = 0;
		}
}

int LTE_init_flag = 0;

int type0_MCU_test_LTE()
{
	if(LTE_init_flag == 0){
		LTE_status = mem909s_msmInit();
	}

		wdgTps_process();
	
	if(LTE_status == 0){
		unsigned char stat[1]={0x02};
		type0_MCU_test_Send(5,1,stat);
	}
	else if(LTE_status == -1){
		unsigned char stat1[1]={0x01};
		type0_MCU_test_Send(5,1,stat1);
	}
	else {
		unsigned char stat0[1]={0x00};
		type0_MCU_test_Send(5,1,stat0);
	}
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

//HEAD		ID			????	data	ROLLCNT  CRT  	 END4
//0X55 66	1BYTE		1BYTE		   			 0~255	          0X0D
// 0    1  2        3        4       5      6         7     

int type0_MCU_test_Send(int id,int len_data,unsigned char *data)
{
	wdgTps_process();
		Read_ROLLCNT++;
		if(Read_ROLLCNT > Read_ROLLCNT_MAX){
			Read_ROLLCNT = 0;
		}
	int send_len;
	unsigned char Send_Crt;
	unsigned char crt;
	if(Dog_flag == 1)
	{
		wdgTps_process();
	}
	int i;
	unsigned char Send_data[200]={0};
	Send_data[0] = Send_Head_0;
	Send_data[1] = Send_Head_1;
	switch(id){
		case 1:{
			Send_data[2]=0x01;
			Send_data[3]=0x01;
			Send_data[4]=data[0];
			Send_data[5]=Send_ROLLCNT;
			Send_Crt = Send_data[0]^Send_data[1];
			for(i = 0;i < 4;i++)
			{
				Send_Crt = Send_Crt^Send_data[i+2];
			}
			Send_data[6]=Send_Crt;
			Send_data[7]=TAIL;
			send_len = 8;
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
					Send_data[len_data+4]=Send_ROLLCNT;
					Send_Crt = Send_data[0]^Send_data[1];
					for(i = 0;i < len_data+4;i++)
					{
						Send_Crt = Send_Crt^Send_data[i+2];
					}
					//strcat(Send_data,num);
					Send_data[len_data+5]=Send_Crt;
					Send_data[len_data+6]=TAIL;
					send_len = len_data+7;
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
			Send_data[5]=Send_ROLLCNT;
			Send_Crt = Send_data[0]^Send_data[1];
			for(i = 0;i < 4;i++)
			{
				Send_Crt = Send_Crt^Send_data[i+2];
			}
			Send_data[6]=Send_Crt;
			Send_data[7]=0x0d;
			send_len = 8;
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
			Send_Crt = Send_data[0]^Send_data[1];
			for(i = 0;i < 4;i++)
			{
				Send_Crt = Send_Crt^Send_data[i+2];
			}
			Send_data[6]=Send_Crt;
			Send_data[7]=0x0d;
			send_len = 8;
			break;
		}
	}
	Send_ROLLCNT++;
	if(Send_ROLLCNT >=Send_ROLLCNT_MAX)
	{
		Send_ROLLCNT = 0;
	}
int sret = uartSendIrq(UART0,Send_data,send_len,&rLenNow);
	if(sret == UART0_SEND_OVER){
		uartSendIrq(UART0,Send_data,send_len,&rLenNow);
	}
	return 0;
}



int flag_1 = 0;
void type0_MCU_Automatic_testing_Start()
{
	if(flag_1 == 0)
	{
		Canread_statust0 = 1;
		flag_1=1;
	}
		wdgTps_process();
	if(Canread_statust0 != 0 || Canread_statust1 != 0){
		Can_read();
	}
	int timeout = 10000,i;
	int flag_f = 0;
	uartReadIrq(UART0,rData,sizeof(rData),&rLenNow);
		for(int i= 0;i < rLenNow;i++)
		{
			if(rData[i] == TAIL)
			{
				rLenNow = 0;
				MCU_test_Analysis(rLenNow);
				uartReadIrqReset(UART0);
				memset(rData,0,sizeof(rData));
				flag_f = 1;
				break;
			}
		}
		
	if(Canread_statust0 != 0 || Canread_statust1 != 0){
		Can_read();
	}
}
