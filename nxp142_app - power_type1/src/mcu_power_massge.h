#ifndef MCU_POWER_MASSGAE_H
#define MCU_POWER_MASSGAE_H
#include <stdbool.h>
struct type0{
	int switch_flag;
	bool TYPE0_ZYNQ ;
	bool TYPE0_VCC_12V_EN ;
	bool TYPE0_4G;
	
};
struct type1{
	int switch_flag;
	bool TYPE1_5EV ;
	bool TYPE1_TX2_SATA;
	bool TYPE1_TX2_NO_SATA;
	bool TYPE1_VCC_12V_EN;
};
struct type2{
	int switch_flag;
	bool TYPE2_ZYNQ ;
	bool TYPE2_VCC_12V_EN ;
	
};
#define SEND_HADE_0 0XA5
#define SEND_HADE_1 0X5A
#define receive_HADE_0 0X5A
#define receive_HADE_1 0XA5
#define receive_TALL 0x0d
#define ON 0x02
#define OFF 0x01

#define type0_send_type1_ID 0x011
#define type0_send_type2_ID 0x012

#define type1_send_type0_ID 0x081
#define type2_send_type0_ID 0x082



#endif