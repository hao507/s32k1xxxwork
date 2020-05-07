/*
 * uart.c
 *
 *  Created on: 2019Äê8ÔÂ2ÈÕ
 *      Author: yxl
 */
#include "gpio.h"
#include "timer.h"
#include "usertype.h"



#define OUT_TIME	50

#define FEED_PORTCATE		PORT_D
#define FEED_PORTNUM		15
int ret = -1;
//PORT_D, 15
int wdgTps_init()
{

	GpioSetDirection(FEED_PORTCATE, FEED_PORTNUM, PORT_OUT);
	GpioSetVal(FEED_PORTCATE, FEED_PORTNUM, PORT_LOW);
	ret = TimerCreate(OUT_TIME);
	if(ret < 0){
		return -1;
	}
	return ret;
}

void wdgTps_process()
{
//	int ret = 0;
//	ret = TimerOutGet(OUT_TIME);
	if(ret >= 0)//timeout
	{
		if(timeOutNum[ret] % 9 == 0){//9
			GpioSetVal(FEED_PORTCATE, FEED_PORTNUM, PORT_HIGH);
		}
		else if(timeOutNum[ret] % 10 == 0){
			GpioSetVal(FEED_PORTCATE, FEED_PORTNUM, PORT_LOW);
			timeOutNum[ret] = 0;
		}
	}
	
}
