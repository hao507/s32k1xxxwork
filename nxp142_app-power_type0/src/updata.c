#include "uart.h"
#include "usertype.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define MCURECV_HEAD			0XAA55
#define CMD_HANDSHAKE			0X01
#define CMD_ASK_UPDATA		0X02
#define CMD_UPDATA				0X03
#define CMD_CHECK					0X04
#define CMD_UPDATA_END		0X05
#define CMD_JUMP_BOOT			0X67
#define MCURECV_END				0X0D

int recvIndex = 0, cmdLen = 0, timeNumRecv = 0, timeNumRecvOld = 0, timeNumSend = 0;

#define UPDATA_HEAD_0 0xAA
#define UPDATA_HEAD_1 0x55
#define UPDATA_CMD 0x67
#define UPDATA_LEN 0x06
#define UPDATA_0 0x11
#define UPDATA_1 0x21
#define UPDATA_2 0x31
#define UPDATA_3 0x41
#define UPDATA_4 0x51
#define UPDATA_5 0x61
#define UPDATA_Hearbeat 
#define UPDATA_END 0X0d
char rdata_buf[200];
int rdata_num = 0;
unsigned char recvData[1024] = {0}, cmdType = 0;
int updata(char *rData,int ret)
{
	int i;
	if(ret > 0){
		for(i=0; i<ret; i++){
			if(rData[i] == (MCURECV_HEAD&0xff) && (recvIndex == 0)){
				recvData[recvIndex++] = rData[i];
			}
			else if(rData[i] == ((MCURECV_HEAD >> 8)&0xff) && (recvIndex == 1)){
				recvData[recvIndex++] = rData[i];
			}
			else if(recvIndex == 2){
				cmdType = rData[i];
				recvData[recvIndex++] = cmdType;
			}
			else if(recvIndex == 3){
				cmdLen = rData[i];
				recvData[recvIndex++] = cmdLen;
			}
			else if(recvIndex>=4 && recvIndex<(4+cmdLen)){
				recvData[recvIndex++] = rData[i];
			}
			else if(recvIndex==4+cmdLen){
				timeNumRecv = recvData[recvIndex++] = rData[i];
			}
			else if(rData[i] == MCURECV_END && recvIndex==(5+cmdLen)){
				timeNumRecv = recvData[recvIndex++] = rData[i];
				
				if(cmdType == CMD_JUMP_BOOT){
					return 1;
				}	
				memset(recvData, 0, sizeof(recvData));
				recvIndex = 0;
			}
			else{
				memset(recvData, 0, sizeof(recvData));
				recvIndex = 0;
			}
		}
	}
/*	char rData[50]={0};
	int rLenNow;
	int timeout = 10000;
	rLenNow = uartReadBlock(UART0,rData,sizeof(rData),timeout);{
		for(int i= 0;i < rLenNow;i++)
		{
			rdata_buf[rdata_num++] = rData[i];
		}
		
		
		
		if(rdata_num >= 11)
		{
			for(int i = 0;i < rdata_num;i++)
			{
					if(rdata_buf[i] == UPDATA_HEAD_0 && rdata_buf[i+1] == UPDATA_HEAD_1)
				{
					int j = i+2;
					if(rdata_buf[j++] == UPDATA_CMD)
						if(rdata_buf[j++] == UPDATA_LEN)
							if(rdata_buf[j++] == UPDATA_0)
								if(rdata_buf[j++] == UPDATA_1)
									if(rdata_buf[j++] == UPDATA_1)
										if(rdata_buf[j++] == UPDATA_2)
											if(rdata_buf[j++] == UPDATA_3)
												if(rdata_buf[j++] == UPDATA_4)
													if(rdata_buf[j++] == UPDATA_5){
															j++;
															if(rdata_buf[j] == UPDATA_END)
																return 1;
													}				
				}
			}
		}
	}*/
	return 0;
}

