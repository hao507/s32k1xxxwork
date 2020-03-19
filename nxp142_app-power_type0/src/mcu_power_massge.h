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

int send_zynq_request_type0();
int receive_zynq_type0(char *power_rdata_type0,int power_rLenNow);
int type0_send_power_list_type2();
int save_type1_power_static();
int type0_send_power_list_type1();
int save_type2_power_static();
int Analysis_zynq_type0(int cmd);
int send_data_Power_list_type0();
void send_init();
int type0_send_type1_request();
int type0_send_type2();
int type0_send_type1_request();
int type0mcu_receive_type1mcu();
int type0mcu_receive_type2mcu();
int save_type1_power_static();
int save_type2_power_static();
int type0_send_power_list_type2();

extern struct type0 send_type_0;
extern struct type0 Save_state_type0;
extern struct type1 send_type_1;
extern struct type1 Save_state_type1;
extern struct type2 send_type_2;
extern struct type2 Save_state_type2; 
extern 	bool send_data_Power_list_type0_flag;
extern 	bool Analysis_zynq_type0_flag;
extern bool type2_readflag;
extern int type1_readflag;
#endif