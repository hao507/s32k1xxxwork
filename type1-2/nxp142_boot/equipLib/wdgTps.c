/*
 * uart.c
 *
 *  Created on: 2019Äê8ÔÂ2ÈÕ
 *      Author: yxl
 */
#include "gpio.h"
#include "timer.h"
#include "usertype.h"

static int timeOutNum = 0;

#define OUT_TIME	50
#define TYPE_SUPPORT	1

#ifdef	TYPE_SUPPORT   
#define FEED_PORTCATE		PORT_D
#define FEED_PORTNUM		15
//#else
//#define FEED_PORTCATE		PORT_D
//#define FEED_PORTNUM		4
#endif
//PORT_D, 15
int wdgTps_init()
{
	int ret = -1;
	GpioSetDirection(FEED_PORTCATE, FEED_PORTNUM, PORT_OUT);
	GpioSetVal(FEED_PORTCATE, FEED_PORTNUM, PORT_LOW);
	ret = TimerCreate(OUT_TIME);
	return ret;
}

void wdgTps_process()
{
	int ret = 0;
	ret = TimerOutGet(OUT_TIME);
	if(ret == 1)//timeout
	{
		timeOutNum++;
		if(timeOutNum % 9 == 0){
			GpioSetVal(FEED_PORTCATE, FEED_PORTNUM, PORT_HIGH);
		}
		else if(timeOutNum % 10 == 0){
			GpioSetVal(FEED_PORTCATE, FEED_PORTNUM, PORT_LOW);
			timeOutNum = 0;
		}
	}
	
}

int oldNum = 0, highFlag = 0;

void wdgTps_noIrq_process(){
	int num = LPTMR_COUNT();
	if(num > oldNum){
		if((num - oldNum >= 450) && (highFlag == 0)){
			GpioSetVal(FEED_PORTCATE, FEED_PORTNUM, PORT_HIGH);
			highFlag = 1;
		}
		else if((num - oldNum >= 500) && (highFlag == 1)){
			GpioSetVal(FEED_PORTCATE, FEED_PORTNUM, PORT_LOW);
			oldNum = num;
			highFlag = 0;
		}
	}
	else if(num < oldNum){
		if((num + 0xffff - oldNum >= 450) && (highFlag == 0)){
			GpioSetVal(FEED_PORTCATE, FEED_PORTNUM, PORT_HIGH);
			highFlag = 1;
		}
		else if((num + 0xffff - oldNum >= 500) && (highFlag == 1)){
			GpioSetVal(FEED_PORTCATE, FEED_PORTNUM, PORT_LOW);
			oldNum = num;
			highFlag = 0;
		}
	}
	
}
//#pragma arm section code = "RAMCODE"
int timeoutNum = 0;
int wdgTps_noIrq_process1(){
	int ret = LPIT0_COUNT();//LPTMR_COUNT();
	if(ret == 1){
		timeoutNum++;
	}
	
	if((timeoutNum >= 14) && (highFlag == 0)){//16
		GpioSetVal(PORT_D, 15, PORT_HIGH);
		highFlag = 1;
		return 1;
	}
	else if((timeoutNum >= 16) && (highFlag == 1) ){//17
		GpioSetVal(PORT_D, 15, PORT_LOW);
		timeoutNum = 0;
		highFlag = 0;
		return 0;
	}
	return -1;
}
//#pragma arm section