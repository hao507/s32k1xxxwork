#ifndef TYPRE1_POWERT_H
#define TYPRE1_POWERT_H

int type1mcu_receive_type0mcu();//接收type0 并初步解析
int type1mcu_send_switch_status_type0mcu();//type1MCU给type0mcu发送打开关闭的状态
int Execution_power_status();//将type0要执行的操作保存到type1_send_type_1中
int send_zynq_request_type1();//询问是否可以执行type1_send_type_1中的操作

int receive_zynq_type1();//接收FPGA的返回内容
int send_FPGA_data_Power_list_type1();//将type1中保存的电源状态发送给FPGA
int Analysis_zynq_type1(int cmd);//对FGPA允许的电源进行操作
int type1_send_type0_Ask_power_list();//向type0 询问电源列表
int Save_power_list();//保存type0发过来的电源列表
int Execution_TYPE1_5EV();
int Execution_TYPE1_TYPE1_VCC_12V_EN();
int Execution_TYPE1_TX2_SATA();
int Execution_TYPE1_TX2_NO_SATA();
int type1_powerinit();
int type1_send_type0_Ask_power_list();
extern int send_data_Power_list_type2_flag;
extern int send_zynq_type1_flag;
extern int type1_send_type0_Ask_power_listflag;
extern struct type0 type1_send_type_0;
extern struct type0 type1_Save_state_type0;
extern struct type1 type1_send_type_1;
extern struct type1 type1_Save_state_type1;
extern struct type2 type1_send_type_2;
extern struct type2 type1_Save_state_type2;
#endif