/*
 * gpio.c
 *
 *  Created on: 2019Äê8ÔÂ6ÈÕ
 *      Author: yxl
 */
#include "uart.h"
#include "timer.h"
#include "usertype.h"
#include "mem909s.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <stdlib.h>
#include "wdgTps.h"
char *AT_CMGS = "AT+CMGS=\"00310037003300380032003000390032003700330037\"\n\r";
static int sendFlag = 0;
const int memRecvTimeout = 2000;
const int ReadyFindTImeout = 500;
//--------------------------------------------------------------------
int  mbstowcs1(register wchar_t *pwcs, register const char *s, int  n)
{
	register int i = n;

	while (--i >= 0) {
		if (!(*pwcs++ = *s++))
			return n - i - 1;
	}
	return n - i;
}


int EncodeUCS2(char *SourceBuf,unsigned char *DestBuf)
{
    unsigned int len,i,j=0;
    wchar_t wcbuf[255];

    setlocale(LC_ALL,"");
    len = mbstowcs1(wcbuf,SourceBuf,sizeof(wcbuf)); /* convert mutibytes string to wide charater string */
    for (i=0;i<len;i++)
    {
        DestBuf[j++] = wcbuf[i]>>8;     /* height byte */
        DestBuf[j++] = wcbuf[i]&0xff;   /* low byte */
    }
    return len*2;
}

int DecodeUCS2(unsigned char *SourceBuf,unsigned char *DestBuf,int len)
{
    wchar_t wcbuf[255];
    int i;

    setlocale(LC_ALL,"");
    for( i=0;i<len/2;i++ ) {
        wcbuf[i]=SourceBuf[2*i];    // height byte
        wcbuf[i]=(wcbuf[i]<<8)+SourceBuf[2*i+1];    // low byte
    }
    return wcstombs(DestBuf,wcbuf,len); /* convert wide charater string to mutibytes string */
}

//----------------------------------------------------------------------------------
enum{
	sendFail,
	sendOver,
	beginFind,
	endOkFind,
	endErrorFind
};

static int msmReadStatus = msmReadNo;
static int sendStatus = -1, msmSendStatus = msmSendNo;

int mem909s_initSend(char *AT_Order) //
{
	char mate_endOk[2]="OK";	
	char mate_endError[5]="ERROR";
	char rData[100]={0};
	int rLenNow = 0;
	int timeout = 10000;
	sendStatus = sendFail;
	
	do{
		uartSendIrq(UART1,(unsigned char*)AT_Order,strlen(AT_Order),&rLenNow);
		if(UART1_SEND_OVER == uartSendStatusGet(UART1))
			break;
			wdgTps_process();
	}while(timeout--);
	if(UART1_SEND_OVER != uartSendStatusGet(UART1)){
		return sendStatus;
	}
	timeout = 50000;
	sendStatus = sendOver;
	do{
		uartReadIrq(UART1,rData,sizeof(rData),&rLenNow);
		if(rLenNow > strlen(AT_Order)){
			for(int i=0; i<rLenNow; i++){
				if(strncmp(rData+i,AT_Order,strlen(AT_Order)) == 0 && (sendStatus==sendOver)){
					sendStatus = beginFind;
				}
				else if(strncmp(mate_endOk,rData+i,strlen(mate_endOk)) == 0 && (sendStatus==beginFind)){
					sendStatus = endOkFind;
					uartReadIrqReset(UART1);
					return sendStatus;
				}
				else if(strncmp(mate_endError,rData+i,strlen(mate_endError)) == 0 && (sendStatus==beginFind)){
					sendStatus = endErrorFind;
					uartReadIrqReset(UART1);
					return sendStatus;
				}
			}
		}
		
			wdgTps_process();
	}while(timeout--);
	
	uartReadIrqReset(UART1);
	return sendStatus;
}

int mem909s_msmSendProcess(char *AT_Order)
{
	char mate_endOk[2]="OK";	
	char mate_endError[5]="ERROR";
	char rData[100]={0};
	int rLenNow = 0, sLenNow = 0, ret = 0;
	static int rStart = 0;
//	char *wData_Error = "send error\n\r";
//	char *AT_CMGS_2 = "4F60597D";
	char *r_AT = "+CMGS:";
	unsigned char 	cz = 0x1A;
	
	if(msmSendStatus == msmSendNo || msmSendStatus == msmSendFail || msmSendStatus == msmSendOk){
		msmSendStatus = msmSendStep1S;
	}
	else if(msmSendStatus == msmSendStep1E){
		msmSendStatus = msmSendStep2S;
	}
	else if(msmSendStatus == msmSendStep2E){
		msmSendStatus = msmSendStep3S;
	}
	switch(msmSendStatus){
		case msmSendStep1S:
			if(sendFlag == 0){
				ret = uartSendIrq(UART1,(unsigned char*)AT_CMGS,strlen(AT_CMGS),&sLenNow);
				if(ret == UART1_SEND_OVER)
					sendFlag = 1;
			}
			uartReadIrq(UART1,rData,sizeof(rData),&rLenNow);
			if(rLenNow > strlen(AT_CMGS)){
				for(int i=0; i< rLenNow; i++){
					if(!strncmp(AT_CMGS, rData+i, strlen(AT_CMGS)) && rStart==0){
						rStart = 1;
					}
					else if((!strncmp(">", rData+i, 1)) && rStart==1){
						msmSendStatus = msmSendStep1E;
						rStart = 0;
						sendFlag = 0;
						uartReadIrqReset(UART1);
					}
					else if(!strncmp(mate_endError, rData+i, strlen(mate_endError))){
						msmSendStatus = msmSendFail;
						sendFlag = 0;
						rStart = 0;
						uartReadIrqReset(UART1);
					}
				}
			}
			break;
		case msmSendStep2S:
			ret = uartSendIrq(UART1,(unsigned char *)AT_Order,strlen(AT_Order),&sLenNow);
			if(ret == UART1_SEND_OVER){
				msmSendStatus = msmSendStep2E;
			}
			break;
		case msmSendStep3S:
			if(sendFlag == 0){
				sendFlag = 1;
				uartSendIrq(UART1,&cz,1,&sLenNow);
			}
			uartReadIrq(UART1,rData,sizeof(rData),&rLenNow);
			for(int i=0; i<rLenNow; i++){
				if(!strncmp(r_AT, rData+i, strlen(r_AT)) && rStart==0){
						rStart = 1;
					}
					else if(!strncmp(mate_endOk, rData+i, strlen(mate_endOk)) && rStart==1){
						msmSendStatus = msmSendOk;
						rStart = 0;
						sendFlag = 0;
						uartReadIrqReset(UART1);
					}
					else if(!strncmp(mate_endError, rData+i, strlen(mate_endError))){
						msmSendStatus = msmSendFail;
						rStart = 0;
						sendFlag = 0;
						uartReadIrqReset(UART1);
					}
			}
			break;
		default:
			break;
	}
	
	return msmSendStatus;
}

int mem909s_msmInit(void)
{
	int ret = -1, timeout = 20, step = 0;
	char *AT_CMGF= "AT+CMGF=1\n\r";
	char *AT_CSMP = "AT+CSMP=,,0,8\n\r";
	char *AT_CSCA= "AT+CSCA=\"8613800731500\",145\n\r"; 
	char *AT_CSCS= "AT+CSCS=\"UCS2\"\n\r";

	do{
	wdgTps_process();
		if(step == 0){
			if(mem909s_initSend(AT_CMGF) == endOkFind)
				step = 1;
		}
		else if(step == 1){
			if(mem909s_initSend(AT_CSMP) == endOkFind)
				step = 2;
		}
		else if(step == 2)
		{
				if(mem909s_initSend(AT_CSCA) == endOkFind)
					step = 3;
		}
		else if(step == 3){
				if(mem909s_initSend(AT_CSCS) == endOkFind){
					ret = 0;
					break;
				}
		}
	}while(timeout--);
	return ret;
}

/*
	firstFlag: 1--if the AT_Orderi is the first time send or we think the first send is timeout; 0--others
	mem909s_msmSend must be call for loop,until msmSendStatus be msmSendOk
*/
int mem909s_msmSend(char *sendData, int len, int firstFlag){
	int ret = msmSendNo;
  char at_oder[50] = {0};
	static char EncodeData[500] = "";
	char EncodeDataMid[200] = "";
	if((sendData==NULL) || ((len >= 50) || (len <= 0)))
		return -1;
	if(firstFlag == 1){
		memset(EncodeDataMid, 0, sizeof(EncodeDataMid));
		EncodeUCS2(sendData,(unsigned char*)at_oder);
		for(int i=0; i<len*2; i++){
        sprintf(EncodeDataMid, "%02x",at_oder[i]);
        strcat(EncodeData, EncodeDataMid);
    }
	}
	ret = mem909s_msmSendProcess(EncodeData);//"4F60597D"
	if(ret == msmSendOk || ret == msmSendFail){
		memset(EncodeDataMid, 0, sizeof(EncodeDataMid));
	}
	return ret;
}

#if 0
static unsigned char msmNum = 0;
int mem909s_msmReadAnysis(char* recvData, int len)
{
	unsigned char lineFeedIndex = 0, msmIndex = 0;
	unsigned char  LineIndex = 0;
	char msmBuf[100] = {0};
	
	if(recvData == NULL || len <= 0 || msmNum<= 0)
		return -1;
	unsigned char lineFeed[100] = {0}, msm[100] = {0};

	for(int i=0; i<len; i++){
		if(strncmp(recvData+i,"\n\r",2) == 0)
			lineFeed[lineFeedIndex++] = i;
		
		if(strncmp(recvData+i,"+CMGL:",6) == 0)
			msm[msmIndex++] = i;
		else if(strncmp(recvData+i,"OK",2) == 0){
			msm[msmIndex++] = i;
		}
	}
	
	for(int i=0; i<msmNum+1; i++){
			for(int j=0; j<lineFeedIndex; j++){
				if(lineFeed[j] > msm[i]){
					strncpy(msmBuf, recvData+lineFeed[j+2], msm[i+1] - lineFeed[j+2]);
					LineIndex = j+2;
					LineIndex= LineIndex;
					/*
						here will wirte msmBuf anysis, but we are not define buf now, so I am not write now;
					*/
				}
			}
	}
	return 0;
}

/*
	mem909s_msmRead must be call for loop,until msmSendStatus be msmSendOk
*/
int mem909s_msmRead(char *Phone_number)
{
	char *AT_CMGD="AT+CMGD=1,4\n\r";
	char rData[200] = {0};
	static char recvbuff[500] = {0};
	static int recvFlag = 0, recvIndex = 0, offset = 0, offsetMid = 0;
	
	int rLenNow = 0;
	char *AT_CMGL="AT+CMGL=\"REC UNREAD\"\n\r";

	if((msmReadStatus == msmReadNo) || (msmReadStatus == msmReadOk) || (msmReadStatus == msmReadFail)){
		uartSendIrq(UART1,(unsigned char*)AT_CMGD,strlen(AT_CMGD),&rLenNow);
		if(uartSendStatusGet(UART1) == UART1_SEND_OVER){
			msmReadStatus = msmReadPrepare;
		}
  }
	else if(msmReadStatus == msmReadPrepare){
		uartSendIrq(UART1,(unsigned char*)AT_CMGL,strlen(AT_CMGL),&rLenNow);
		if(uartSendStatusGet(UART1) == UART1_SEND_OVER){
			msmReadStatus = msmReadStart;
			uartReadIrq(UART1,rData,sizeof(rData),&rLenNow);
			TimerCreate(memRecvTimeout);
		}
	}
	else if(msmReadStatus == msmReadStart){
		int readStatus = uartReadIrq(UART1,rData,sizeof(rData),&rLenNow);
		int i = 0;
		if(offset > rLenNow){
			offset = offset;
		}
		int timeoutFlag = TimerOutGet(memRecvTimeout);
		if((timeoutFlag == TIMER_OUT) && (recvFlag == 0)){
				TimerDelete(memRecvTimeout);
				msmReadStatus = msmReadPrepare;
				uartReadIrqReset(UART1);
			  memset(recvbuff, 0, sizeof(recvbuff));
			  recvIndex = 0;
			  offset = 0;
			  offsetMid =0;
				return msmReadStatus;
		}
		for(i=offset; i<rLenNow; i++){
			if(!strncmp(rData+i,"+CMGL:",6)){
				msmNum++;
			}
			if((!strncmp(rData+i,"+CMGL:",6)) && (recvFlag == 0)){
				recvFlag = 1;
			}
			else if((!strncmp(rData+i,"OK",2)) && (recvFlag == 1)){
				recvFlag = 2;
			}
			else if(!strncmp(rData+i,"ERROR",5)){
				offsetMid = i+5;
				recvFlag = 0;
				msmReadStatus = msmReadFail;
				memset(recvbuff, 0, sizeof(recvbuff));
				recvIndex = 0;
				break;
			}
			if((recvFlag == 1)){
				recvbuff[recvIndex] = rData[i];
				recvIndex = recvIndex + 1;
				offsetMid = i+1;
			}
			else if(recvFlag == 2){
				recvbuff[recvIndex + 1] = rData[i];
				recvbuff[recvIndex + 2] = rData[i+1];
				offsetMid = i+1;
				mem909s_msmReadAnysis(recvbuff, recvIndex);
				memset(recvbuff, 0, sizeof(recvbuff));
				recvIndex = 0;
				msmReadStatus = msmReadOk;
				recvFlag = 0;
			}
			else if(recvIndex > 450){
				offsetMid = i+1;
				recvFlag = 0;
				msmReadStatus = msmReadFail;
				memset(recvbuff, 0, sizeof(recvbuff));
				recvIndex = 0;
			}
		}
		if((msmReadStatus == msmReadOk) || (msmReadStatus == msmReadFail)){
			uartReadIrqReset(UART1);
			memset(recvbuff, 0, sizeof(recvbuff));
			recvIndex = 0;
			offset = 0;
			offsetMid =0;
		}
		else{
			offset = offsetMid;
		}
		if(UART1_READ_OVER == readStatus){
			offset = offsetMid = 0;
		}
	}
	return msmReadStatus;
}
#else//-------------------
static unsigned char msmNum = 0;
int mem909s_msmReadAnysis(char* recvData, int len)
{
	unsigned char lineFeedIndex = 0;
	char msmBuf[100] = {0};
	
	if(recvData == NULL || len <= 0)
		return -1;
	unsigned char lineFeed[10] = {0};

	for(int i=0; i<len; i++){
		if(strncmp(recvData+i,"\n\r",2) == 0)
			lineFeed[lineFeedIndex++] = i;
	}
	if(len - lineFeed[1] > 0){
		strncpy(msmBuf, recvData+lineFeed[0], len - lineFeed[1]);
	}
	else{
		return -1;
	}
	return 0;
}

int mem909s_msmRead(char *Phone_number)
{
	char *AT_CMGF= "AT+CMGF=1\n\r";
	char *AT_CMGD="AT+CMGD=1,4\n\r";
	char rData[200] = {0};
	static char recvbuff[500] = {0};
	static int recvFlag = 0, recvIndex = 0, offset = 0, offsetMid = 0;
	static prepreFlag = 0;
	
	int rLenNow = 0;
	char *AT_CMGL="AT+CMGL=\"REC UNREAD\"\n\r";

	if((msmReadStatus == msmReadNo) || (msmReadStatus == msmReadOk) || (msmReadStatus == msmReadFail)){
		if(prepreFlag == 0)
			uartSendIrq(UART1,(unsigned char*)AT_CMGF,strlen(AT_CMGF),&rLenNow);
		else if(prepreFlag == 1){
			uartSendIrq(UART1,(unsigned char*)AT_CMGD,strlen(AT_CMGD),&rLenNow);
			if(uartSendStatusGet(UART1) == UART1_SEND_OVER){
				msmReadStatus = msmReadPrepare;
			}
		}
		if(uartSendStatusGet(UART1) == UART1_SEND_OVER){
				prepreFlag = 1;
		}
//			uartSendIrq(UART1,(unsigned char*)AT_CMGD,strlen(AT_CMGD),&rLenNow);
//			if(uartSendStatusGet(UART1) == UART1_SEND_OVER){
//				msmReadStatus = msmReadPrepare;
//			}
  }
	else if(msmReadStatus == msmReadPrepare){
		uartSendIrq(UART1,(unsigned char*)AT_CMGL,strlen(AT_CMGL),&rLenNow);
		if(uartSendStatusGet(UART1) == UART1_SEND_OVER){
			msmReadStatus = msmReadStart;
			uartReadIrq(UART1,rData,sizeof(rData),&rLenNow);
			TimerCreate(memRecvTimeout);
		}
	}
	else if(msmReadStatus == msmReadStart){
		int readStatus = uartReadIrq(UART1,rData,sizeof(rData),&rLenNow);
		int i = 0;
		if(offset > rLenNow){
			offset = offset;
		}
		int timeoutFlag = TimerOutGet(memRecvTimeout);
		if((timeoutFlag == TIMER_OUT) && (recvFlag == 0)){
				TimerDelete(memRecvTimeout);
				msmReadStatus = msmReadPrepare;
				uartReadIrqReset(UART1);
			  memset(recvbuff, 0, sizeof(recvbuff));
			  recvIndex = 0;
			  offset = 0;
			  offsetMid =0;
				return msmReadStatus;
		}
		for(i=offset; i<rLenNow; i++){
			if((!strncmp(rData+i,"+CMGL:",6)) && (recvFlag == 0)){
				recvFlag = 1;
			}
			if((recvFlag == 1)){
				recvbuff[recvIndex] = rData[i];
				recvIndex = recvIndex + 1;
				offsetMid = i+1;
			}
		}
		//---------------------------------
		unsigned char cmglNum = 0;
		if(recvIndex > 450){
				recvFlag = 0;
				msmReadStatus = msmReadFail;
				memset(recvbuff, 0, sizeof(recvbuff));
				recvIndex = 0;
		}
		for(int i=0; i< recvIndex; i++){
			if(!strncmp(recvbuff+i,"+CMGL:",6)){
				cmglNum++;
			}
			if((!strncmp(recvbuff+i,"OK",2)) || (cmglNum > 1)){
				cmglNum = 0;
				mem909s_msmReadAnysis(recvbuff, i);
				memcpy(recvbuff, recvbuff+i, sizeof(recvbuff) - i);
				recvIndex -= i;
				i = 0;
				if(!strncmp(recvbuff+i,"OK",2)){
					memset(recvbuff, 0, sizeof(recvbuff));
					recvIndex = 0;
					msmReadStatus = msmReadOk;
					recvFlag = 0;
					break;
				}
			}
			else if(!strncmp(recvbuff+i,"ERROR",5)){
				recvFlag = 0;
				msmReadStatus = msmReadFail;
				memset(recvbuff, 0, sizeof(recvbuff));
				recvIndex = 0;
				break;
			}
		}
		//-----------------------------------------------
		if((msmReadStatus == msmReadOk) || (msmReadStatus == msmReadFail)){
			uartReadIrqReset(UART1);
			offset = 0;
			offsetMid =0;
		}
		else{
			offset = offsetMid;
		}
		if(UART1_READ_OVER == readStatus){
			offset = offsetMid = 0;
		}
	}
	return msmReadStatus;
}
#endif
/*
	0--not ready
	1--ready
*/
int mem909sReadyGet(void){
	char* at ="AT\n\r";
	unsigned char rData[100] = {0};
	int rLenNow = 0, ret = 0;
	static int readySendFlag = 0;
	int timeout = 50000;
	if(!readySendFlag){
		ret = TimerCreate(ReadyFindTImeout);
		if(ret == -1)
			return -1;
		readySendFlag = 1;
		uartSendIrq(UART1,(unsigned char*)at,strlen(at),&rLenNow);
	}
	uartReadIrq(UART1,rData,sizeof(rData),&rLenNow);
	while(timeout--){
		wdgTps_process();
	};
	for(int i=0; i<rLenNow; i++){
		if(strncmp(rData, "OK", 2)==0){
			readySendFlag = 0;
			TimerDelete(ReadyFindTImeout);
			uartReadIrqReset(UART1);
			return 1;
		}
	}
	wdgTps_process();
	ret = TimerOutGet(ReadyFindTImeout);
	if(ret == TIMER_OUT){
		readySendFlag = 0;
	}
	return 0;
}

int memSendStatusGet(void)
{
	return msmSendStatus;
}

int memReadStatusGet(void)
{
	return msmReadStatus;
}

int ceshi(unsigned char* sendData1, unsigned char* sendData2, int len){
	unsigned char at_oder[100] = {0}, decodeData[100] = {0}; 
	
	EncodeUCS2(sendData1,(unsigned char*)at_oder);	
	DecodeUCS2(sendData2, decodeData, len);
	return 0;
}

int Low_Power_Consumption()
{
	int r = mem909s_msmInit();
	int step = 0,ret = 1;
	char * AT_WAKEUPCFG = "AT^WAKEUPCFG=1,1,2\n\r";
	char *AT_SLEEPCFG= "AT^SLEEPCFG=1,8\n\r";
	int timeout = 5000;
	if(0 == r){
			do{
		if(step == 0){
			if(mem909s_initSend(AT_WAKEUPCFG) == endOkFind)
				step = 1;
		}
		else if(step == 1){
			if(mem909s_initSend(AT_SLEEPCFG) == endOkFind)
				ret = 0;
			break;
		}
	}while(timeout--);
	}
	return ret;
}
	
	
