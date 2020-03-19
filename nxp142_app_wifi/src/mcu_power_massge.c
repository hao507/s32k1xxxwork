#include "mcu_power_massge.h"
#include "canbus.h"
#include "uart.h"

struct type0 send_type_0;
struct type0 Save_state_type0;
struct type1 send_type_1;
struct type1 Save_state_type1;
struct type2 send_type_2;
struct type2 Save_state_type2; 

int send_num_FPGA = 0x00;
int send_num_MCU = 0X00;
char power_rdata_type0[100]={0};
int type0_data_strat = 0x00;
int type0_data_end = 0x00;


int send_zynq_request_type0();
int receive_zynq_type0();
int type0_send_power_list_type2();
int save_type1_power_static();
int type0_send_power_list_type1();
int save_type2_power_static();
int Execution_TYPE0_TYPE0_VCC_12V_EN();
int Execution_TYPE0_TYPE0_4G();
int Execution_TYPE0_ZYNQ();
int Analysis_zynq_type0();
int send_data_Power_list_type0();

char can_rID;
unsigned char can_rdata[8]={0};
bool  Analysis_zynq_type0_flag = 0;
bool type2_readflag = 0;
void send_init()
{
	memset(&send_type_0,0x00,sizeof(struct type0));
	memset(&Save_state_type0,0x00,sizeof(struct type0));
	memset(&send_type_1,0x00,sizeof(struct type1));
	memset(&Save_state_type1,0x00,sizeof(struct type1));
	memset(&send_type_2,0x00,sizeof(struct type2));
	memset(&Save_state_type2,0x00,sizeof(struct type2));
}
int type0_send_type1_request()//type0 mcu与type1 can的发送函数 一样要设置send_type_1里的值
{
	char send_data[8]={0};
	char send_ID = type0_send_type1_ID;
	send_data[0] = 0x01;
	send_data[1] = send_num_MCU++;
	send_data[2] = send_type_1.switch_flag;
	char data = 0x00;
	send_data[3] = data | send_type_1.TYPE1_5EV;
	send_data[3] = data | (send_type_1.TYPE1_VCC_12V_EN << 1);
	send_data[3] = data | (send_type_1.TYPE1_TX2_SATA << 3);
	send_data[3] = data | (send_type_1.TYPE1_TX2_NO_SATA << 4);
  Canbus1Send(send_ID, (unsigned char *)send_data, sizeof(send_data),CAN_STANDARD_MODE);
}
	
int type0_send_type2()
{
	char send_data[8]={0};
	char send_ID = type0_send_type2_ID;
	send_data[0] = 0x01;
	send_data[1] = send_num_MCU++;
	send_data[2] = send_type_2.switch_flag;
	char data = 0x00;
	send_data[3] = data | send_type_2.TYPE2_ZYNQ;
	send_data[3] = data | (send_type_2.TYPE2_VCC_12V_EN << 1);
	Canbus1Send(send_ID, (unsigned char *)send_data, sizeof(send_data),CAN_STANDARD_MODE);
}
	

int type0mcu_receive_type1mcu()
{
	int queueLen = 0;
	canbus1ReadIrq((int *)&can_rID, can_rdata,&queueLen);
	if(queueLen > 0){
		if(type1_send_type0_ID == can_rID){
		if(can_rdata[0] == 0x01){//回馈电源关闭/打开
			save_type1_power_static();
			
		}
		else if(can_rdata[1] == 0x02){
			type0_send_power_list_type1();
		}
	}
	}

}

int type0mcu_receive_type2mcu()
{
	int queueLen = 0;
	canbus1ReadIrq((int *)&can_rID, can_rdata,&queueLen);
	if(type2_send_type0_ID == can_rID){
		if(can_rdata[0] == 0x01){//回馈电源关闭/打开
			save_type2_power_static();
			type2_readflag = 1;
		}
		else if(can_rdata[0] == 0x02){
			type0_send_power_list_type2();
		}
	}
}


int save_type1_power_static()
{
	if(ON == can_rdata[2]){
		if(can_rdata[3] & 1){
			Save_state_type1.TYPE1_5EV = 1;
		}
		if((can_rdata[3] >> 1) & 1){
			Save_state_type1.TYPE1_VCC_12V_EN = 1;
		}
		if((can_rdata[3] >> 3) & 1){
			Save_state_type1.TYPE1_TX2_SATA = 1;
		}
		if((can_rdata[3] >> 4) & 1){
			Save_state_type1.TYPE1_TX2_NO_SATA = 1;
		}
	}
	else if(OFF == can_rdata[2]){
		if(can_rdata[3] & 1){
			Save_state_type1.TYPE1_5EV = 0;
		}
		if((can_rdata[3] >> 1) & 1){
			Save_state_type1.TYPE1_VCC_12V_EN = 0;
		}
		if((can_rdata[3] >> 3) & 1){
			Save_state_type1.TYPE1_TX2_SATA = 0;
		}
		if((can_rdata[3] >> 4) & 1){
			Save_state_type1.TYPE1_TX2_NO_SATA = 0;
		}
	}
}
int type0_send_power_list_type1()
{

	char sendbuf[8];
	char sendID = type0_send_type1_ID;
	char data0 = 0x00;
	char data1 = 0x00;
	char data2 = 0x00;
	sendbuf[0] = 0x02;
	sendbuf[1] = send_num_MCU++;
	
	data0 = data0 | Save_state_type0.TYPE0_ZYNQ ;
	data0 = data0 | Save_state_type0.TYPE0_4G << 1;
	data0 = data0 | Save_state_type0.TYPE0_VCC_12V_EN << 2;
	
	data1 = data1 | Save_state_type1.TYPE1_5EV;
	data1 = data1 | Save_state_type1.TYPE1_TX2_SATA << 1;
	data1 = data1 | Save_state_type1.TYPE1_TX2_NO_SATA << 2;
	data1 = data1 | Save_state_type1.TYPE1_VCC_12V_EN << 3;
	
	data2 = data2 | Save_state_type2.TYPE2_ZYNQ;
	data2 = data2 | Save_state_type2.TYPE2_VCC_12V_EN << 1;
	sendbuf[2] = data0;
	sendbuf[3] = data1;
	sendbuf[4] = data2;
	sendbuf[7] = 0x01;
	Canbus1Send(sendID, (unsigned char *)sendbuf, sizeof(sendbuf),CAN_STANDARD_MODE);
}
int save_type2_power_static()
{
	if(ON == can_rdata[2]){
		if(can_rdata[3] & 1){
			Save_state_type2.TYPE2_ZYNQ = 1;
		}
		if((can_rdata[3] >> 1) & 1){
			Save_state_type2.TYPE2_VCC_12V_EN = 1;
		}
	}
	else if(OFF == can_rdata[2]){
		if(can_rdata[3] & 1){
			Save_state_type2.TYPE2_ZYNQ = 0;
		}
		if((can_rdata[3] >> 1) & 1){
			Save_state_type2.TYPE2_VCC_12V_EN = 0;
		}
	}
}
int type0_send_power_list_type2()
{
	char sendbuf[8];
	char sendID = type0_send_type2_ID;
	char data0 = 0x00;
	char data1 = 0x00;
	char data2 = 0x00;
	sendbuf[0] = 0x02;
	sendbuf[1] = send_num_MCU++;
	
	data0 = data0 | Save_state_type0.TYPE0_ZYNQ ;
	data0 = data0 | Save_state_type0.TYPE0_4G << 1;
	data0 = data0 | Save_state_type0.TYPE0_VCC_12V_EN << 2;
	data1 = data1 | Save_state_type1.TYPE1_5EV;
	data1 = data1 | Save_state_type1.TYPE1_TX2_SATA << 1;
	data1 = data1 | Save_state_type1.TYPE1_TX2_NO_SATA << 2;
	data1 = data1 | Save_state_type1.TYPE1_VCC_12V_EN << 3;
	data2 = data2 | Save_state_type2.TYPE2_ZYNQ;
	data2 = data2 | Save_state_type2.TYPE2_VCC_12V_EN << 1;
	sendbuf[2] = data0;
	sendbuf[3] = data1;
	sendbuf[4] = data2;
	sendbuf[7] = 0x01;
	Canbus1Send(sendID, (unsigned char *)sendbuf, sizeof(sendbuf),CAN_STANDARD_MODE);
}

int receive_zynq_type0()//接收
{
	int power_rLenNow = 0;
	int flag = 0;
	uartReadIrq(UART0,(unsigned char *)power_rdata_type0,sizeof(power_rdata_type0),&power_rLenNow);
		for(int i= 0;i < power_rLenNow;i++){
			if(power_rdata_type0[i] == receive_HADE_0){
				flag = 1;
				type0_data_strat = i;
			}
			else if(power_rdata_type0[i] == receive_TALL){
				flag = 2;
				type0_data_end = i;
				if(power_rdata_type0[type0_data_strat+2] == 0x02){
						send_data_Power_list_type0();
						memset(power_rdata_type0,0,sizeof(power_rdata_type0));
						uartReadIrqReset(UART0);
				}
				if(power_rdata_type0[type0_data_strat+2] == 0x01){
						Analysis_zynq_type0_flag = 1;
						Analysis_zynq_type0();//如果cmd为1 保存电源状态			
						memset(power_rdata_type0,0,sizeof(power_rdata_type0));
						uartReadIrqReset(UART0);
					}
				}
			}
}


int send_zynq_request_type0()//打开关闭电源 switch_flag表示为ON表示打开 OFF表示关闭 send_type_0.xxx为1表示要操作这个电源为0表示不操作
{/*
	eg: send_type_0 = ON;
	send_type_0.TYPE0_ZYNQ = 1;
	send_type_0.TYPE0_VCC_12V_EN = 0;
*/
	char crt = 0x00;
	char send_data[10]={0};
	send_data[0] = SEND_HADE_0;
	send_data[1] = SEND_HADE_1;
	send_data[2] = 0x01;
	send_data[3] = send_num_FPGA++;;
	send_data[4] = 0x03;
	send_data[5] = send_type_0.switch_flag;
	char data = 0x00;
	data = data | send_type_0.TYPE0_ZYNQ;
	data = data |(send_type_0.TYPE0_VCC_12V_EN << 1);
	data = data |(send_type_0.TYPE0_4G << 2);
	send_data[6] = data;
	send_data[7] = 0x00;
	crt = send_data[0] ^ send_data[1];
	for(int i = 2; i < 8;i++){
		crt = crt ^ send_data[i];
	}
	send_data[8] = crt;
	send_data[9] = 0x0d;
	int sLen;
	int sret =  uartSendIrq(UART0,(unsigned char *)send_data,10,&sLen);
	if(sret == UART1_SEND_OVER){
				 uartSendIrq(UART0,(unsigned char *)send_data,10,&sLen);
	}

}

int Analysis_zynq_type0()//解析关闭电源是否允许关闭
{

	int crt;
	char data_buf[20]={0};
	int b = 0;
	for(int i = type0_data_strat;i <=type0_data_end;i++){
		data_buf[b++] = power_rdata_type0[i];
	}
	crt = data_buf[0] ^ data_buf[1];
	for(int i = 2;i < 7;i++){
		crt = crt ^ data_buf[i];
	}
	if(crt != power_rdata_type0[type0_data_end-1]){
		return -1;
	}
	
	if(data_buf[0] == receive_HADE_0 && data_buf[1] == receive_HADE_1){
		if(data_buf[2] == 0x01){
			if(data_buf[5] == ON){
				if(data_buf[6] & 1){
					Save_state_type0.TYPE0_ZYNQ = 1;
					Execution_TYPE0_ZYNQ();
				}
				 if(data_buf[6] >> 1 & 1){
					Save_state_type0.TYPE0_VCC_12V_EN = 1;
					Execution_TYPE0_TYPE0_VCC_12V_EN();
				}
				 if(data_buf[6]  >> 2 & 1){
					Save_state_type0.TYPE0_4G = 1;
					Execution_TYPE0_TYPE0_4G();
				}
					
			}
			else if(data_buf[5] == OFF){
				if(data_buf[6] & 1){
					Save_state_type0.TYPE0_ZYNQ = 0;
					Execution_TYPE0_ZYNQ();
				}
				 if(data_buf[6]  >> 1 & 1){
					Save_state_type0.TYPE0_VCC_12V_EN = 0;
					Execution_TYPE0_TYPE0_VCC_12V_EN();
				}
				 if(data_buf[6]  >> 2 & 1){
					Save_state_type0.TYPE0_4G = 0;
					Execution_TYPE0_TYPE0_4G();
				}
			}
		}
	}
}
bool send_data_Power_list_type0_flag = 0;


int send_data_Power_list_type0()//发送电源列表
{
	char send_data1[30]={0};
	send_data1[0] = SEND_HADE_0;
	send_data1[1] = SEND_HADE_1;
	send_data1[2] = 0x02;
	send_data1[3] = send_num_FPGA++;
	send_data1[4] = 0x06;
	char data0 = 0x00,data1 = 0x00,data2 = 0x00;
	data0 = data0 | Save_state_type0.TYPE0_ZYNQ;
	data0 = data0 | Save_state_type0.TYPE0_4G<< 1;
	data0 = data0 | Save_state_type0.TYPE0_VCC_12V_EN<< 2;
	
	data1 = data1 | Save_state_type1.TYPE1_5EV;
	data1 = data1 | Save_state_type1.TYPE1_TX2_SATA << 1;
	data1 = data1 | Save_state_type1.TYPE1_TX2_NO_SATA  << 2;
	data1 = data1 | Save_state_type1.TYPE1_VCC_12V_EN << 3;
	
	data2 = data2 | Save_state_type2.TYPE2_ZYNQ;
	data2 = data2 | Save_state_type2.TYPE2_VCC_12V_EN << 1;
	
	
	send_data1[5] = data0;
	send_data1[6] = data1;
	send_data1[7] = data2;
	send_data1[8] = 0x00;
	send_data1[9] = 0x00;
	send_data1[10] = 0x01;
	char crt1 = 0x00;
	crt1 = send_data1[0] ^ send_data1[1];
	for(int i = 2;i < 11;i++){
		crt1 = crt1 ^ send_data1[i];
	}
	send_data1[11] = crt1;
	send_data1[12] = 0x0d;
	int rLenNow;
	int sret = uartSendIrq(UART0,(unsigned char *)send_data1,sizeof(send_data1),&rLenNow);
	if(sret == UART0_SEND_OVER){
					uartSendIrq(UART0,(unsigned char *)send_data1,sizeof(send_data1),&rLenNow);
	}
}

int Execution_TYPE0_ZYNQ()
{
	if(Save_state_type0.TYPE0_ZYNQ == 1){
		
	}
	else if(Save_state_type0.TYPE0_ZYNQ == 0){
		
	}
}
int Execution_TYPE0_TYPE0_VCC_12V_EN()
{
	if(Save_state_type0.TYPE0_VCC_12V_EN == 1){
		
	}
	else if(Save_state_type0.TYPE0_VCC_12V_EN == 0){
		
	}
}
int Execution_TYPE0_TYPE0_4G()
{
	if(Save_state_type0.TYPE0_4G == 1){
		
	}
	else if(Save_state_type0.TYPE0_4G == 0){
		
	}
}


	
	
	

	 
	
	
	
	















