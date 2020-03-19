#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "winc1500_driver_config.h"
#include "wf_types.h"
#include "wf_socket.h"
#include "wf_utils.h"
#include "wf_errors.h"
#include "wf_ota.h"
#include "gpio.h"
#include "usertype.h"
#include "winc1500_api.h"
//void m2mStub_PinSet_CE(t_m2mWifiPinAction action)
//{
//	
//	if (action == M2M_WIFI_PIN_LOW)
//	{
//		GpioSetVal(PORT_C,16,PORT_LOW);
//	}
//	else
//	{
//		GpioSetVal(PORT_C,16,PORT_HIGH);
//	}
//}
//void m2mStub_PinSet_RESET(t_m2mWifiPinAction action)
//{
//		if (action == M2M_WIFI_PIN_LOW)
//	{
//		GpioSetVal(PORT_C,17,PORT_LOW);
//	}
//	else
//	{
//		GpioSetVal(PORT_C,17,PORT_HIGH);
//	}
//}
//void m2mStub_PinSet_SPI_SS(t_m2mWifiPinAction action)
//{
//	if (action == M2M_WIFI_PIN_LOW)
//	{
//		GpioSetVal(PORT_C,17,PORT_LOW);
//	}
//	else
//	{
//		GpioSetVal(PORT_C,17,PORT_HIGH);
//	}
//}