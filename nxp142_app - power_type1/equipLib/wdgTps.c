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

#ifdef	TYPE_SUPPORT
#define FEED_PORTCATE		PORT_D
#define FEED_PORTNUM		15
#else
#define FEED_PORTCATE		PORT_D
#define FEED_PORTNUM		4
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
