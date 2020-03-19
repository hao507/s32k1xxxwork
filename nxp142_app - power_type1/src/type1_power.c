#include "mcu_power_massge.h"
#include "canbus.h"
#include "uart.h"
#include "type1_power.h"
#include "gpio.h"
struct type0 type1_send_type_0;
struct type0 type1_Save_state_type0;
struct type1 type1_send_type_1;
struct type1 type1_Save_state_type1;
struct type2 type1_send_type_2;
struct type2 type1_Save_state_type2;
char send_num_FPGA = 0x00;
char send_num_MCU = 0X00;
int send_zynq_type1_flag = 0;
int save_list_flag = -1;

char power_rdata_type1[100]={0};


int type1_data_strat = 0;
int type1_data_end = 0;


char can_rID;
unsigned char can_rdata[8]={0};
int send_data_Power_list_type2_flag = -1;
int type1mcu_receive_type0mcu();//接收type0 并初步解析
int type1mcu_send_switch_status_type0mcu();//type1MCU给type0mcu发送打开关闭的状态
int Execution_power_status();//将type0要执行的操作保存到type1_send_type_1中
int send_zynq_request_type1();//询问是否可以执行type1_send_type_1中的操作

int receive_zynq_type1();//接收FPGA的返回内容
int send_FPGA_data_Power_list_type1();//将type1中保存的电源状态发送给FPGA
int Analysis_zynq_type1();//对FGPA允许的电源进行操作
int type1_send_type0_Ask_power_list();//向type0 询问电源列表
int Save_power_list();//保存type0发过来的电源列表
int Execution_TYPE1_5EV();
int Execution_TYPE1_TYPE1_VCC_12V_EN();
int Execution_TYPE1_TX2_SATA();
int Execution_TYPE1_TX2_NO_SATA();
int type1_send_type0_Ask_power_listflag = -1;

int type1_powerinit()
{
	memset(&type1_send_type_0,0,sizeof(struct type0));
	memset(&type1_Save_state_type0,0,sizeof(struct type0));
	memset(&type1_send_type_1,0,sizeof(struct type1));
	memset(&type1_Save_state_type1,0,sizeof(struct type1));
	memset(&type1_send_type_2,0,sizeof(struct type2));
	memset(&type1_Save_state_type2,0,sizeof(struct type2));
}
int Execution_TYPE1_5EV(int val)
{
	if(ON == val){
	GpioSetVal(PORT_D, 4, PORT_HIGH); 
		
	}
	else if(OFF == val){
		GpioSetVal(PORT_D, 4, PORT_LOW); 
	}
}
int Execution_TYPE1_TYPE1_VCC_12V_EN(int val)
{
	if(ON == val){
	GpioSetVal(PORT_D, 3, PORT_HIGH); 
		
	}
	else if(OFF == val){
		GpioSetVal(PORT_D, 3, PORT_LOW); 
	}
}
int Execution_TYPE1_TX2_SATA(int val)
{
		if(ON == val){
	GpioSetVal(PORT_D, 5, PORT_HIGH); 
		
	}
	else if(OFF == val){
		GpioSetVal(PORT_D, 5, PORT_LOW); 
	}
}
int Execution_TYPE1_TX2_NO_SATA(int val)
{
	if(ON == val){
	GpioSetVal(PORT_D, 6, PORT_HIGH); 
		
	}
	else if(OFF == val){
		GpioSetVal(PORT_D, 6, PORT_LOW); 
	}
}

int type1_send_type0_Ask_power_list()
{
	
    char send_Id = type1_send_type0_ID;
    char send_buf[8]={0};
    send_buf[0] = 0x02;
    send_buf[1] = send_num_MCU++;
    send_buf[2] = 0x02;
    Canbus0Send(send_Id,(unsigned char *)send_buf,sizeof (send_buf), CAN_STANDARD_MODE);
}
int Save_power_list()
{

		type1_send_type0_Ask_power_listflag = 2;
    type1_Save_state_type0.TYPE0_ZYNQ =can_rdata[2] & 1;
    type1_Save_state_type0.TYPE0_4G = can_rdata[2] >> 1 & 1;
    type1_Save_state_type0.TYPE0_VCC_12V_EN = can_rdata[2] >> 2 & 1;
    type1_Save_state_type1.TYPE1_5EV = can_rdata[3] & 1;
    type1_Save_state_type1.TYPE1_TX2_SATA = can_rdata[3] >> 1 & 1;
    type1_Save_state_type1.TYPE1_TX2_NO_SATA = can_rdata[3] >> 2 & 1;
    type1_Save_state_type1.TYPE1_VCC_12V_EN = can_rdata[3] >> 3 & 1;
    //type1_Save_state_type2.TYPE2_ZYNQ = can_rdata[4] & 1;
   // type1_Save_state_type2.TYPE2_VCC_12V_EN = can_rdata[4] >> 1 & 1;


}

int type1mcu_receive_type0mcu()//can接收id 为can_rID  地址为can_rdata
{
	int qul = 0;
	canbus0ReadIrq((int*)&can_rID,can_rdata,&qul);
		if(type0_send_type1_ID == can_rID){
				if(can_rdata[0] == 0x01){//请求电源关闭/打开
					Execution_power_status();//解析type0 mcu 的操作
					send_zynq_type1_flag = 1;
					send_zynq_request_type1();//询问FPGA是否允许关闭
				//	receive_zynq_type1();//接收并保存执行   应该放在循环里面
				}
				else if(can_rdata[0] == 0x02){
						Save_power_list();
					if(save_list_flag == 2){
					send_FPGA_data_Power_list_type1();
					}
				}
			}
}

int type1mcu_send_switch_status_type0mcu()//返回状态
{
	char sendbuf[8];
	char sendID = type1_send_type0_ID;
	char data0 = 0x00;
	char data1 = 0x00;
	char data2 = 0x00;
	sendbuf[0] = 0x01;
	sendbuf[1] = send_num_MCU++;
	
	data1 = data1 | type1_Save_state_type1.TYPE1_5EV;
	data1 = data1 | type1_Save_state_type1.TYPE1_TX2_SATA << 1;
	data1 = data1 | type1_Save_state_type1.TYPE1_TX2_NO_SATA << 2;
	data1 = data1 | type1_Save_state_type1.TYPE1_VCC_12V_EN << 3;
	
	sendbuf[2] = type1_Save_state_type1.switch_flag;
	sendbuf[3] = data1;
	sendbuf[4] = 0x00;
	sendbuf[7] = 0x01;
	Canbus0Send(sendID,(unsigned char *)sendbuf,sizeof (sendbuf), CAN_STANDARD_MODE);
}


int Execution_power_status()
{
	if(ON == can_rdata[2]){
		type1_send_type_1.switch_flag = ON;
		if(can_rdata[3] & 1){
			type1_send_type_1.TYPE1_5EV = 1;
		}
		if((can_rdata[3] >> 1)& 1){
			type1_send_type_1.TYPE1_VCC_12V_EN = 1;
		}
		if((can_rdata[3] >> 3)& 1){
			type1_send_type_1.TYPE1_TX2_SATA = 1;
		}
		if((can_rdata[3] >> 4)& 1){
			type1_send_type_1.TYPE1_TX2_NO_SATA = 1;
		}
	}
	else if(OFF == can_rdata[2]){
		type1_send_type_1.switch_flag = OFF;
		if(can_rdata[3] & 1){
			type1_send_type_1.TYPE1_5EV = 1;
		}
		if((can_rdata[3] >> 1)& 1){
			type1_send_type_1.TYPE1_VCC_12V_EN = 1;
		}
		if((can_rdata[3] >> 3)& 1){
			type1_send_type_1.TYPE1_TX2_SATA = 1;
		}
		if((can_rdata[3] >> 4)& 1){
			type1_send_type_1.TYPE1_TX2_NO_SATA = 1;
		}
	}
}

int send_zynq_request_type1()//打开关闭电源 switch_flag表示为ON表示打开 OFF表示关闭 type1_send_type_0.xxx为1表示要操作这个电源为0表示不操作
{/*
	eg: type1_send_type_0 = ON;
	type1_send_type_0.TYPE0_ZYNQ = 1;
	type1_send_type_0.TYPE0_VCC_12V_EN = 0;
*/
	char crt = 0x00;
	char send_data[10]={0};
	send_data[0] = SEND_HADE_0;
	send_data[1] = SEND_HADE_1;
	send_data[2] = 0x01;
	send_data[3] = send_num_FPGA++;
	send_data[4] = 0x03;
	send_data[5] = type1_send_type_1.switch_flag;
	char data = 0x00;
	data = data | type1_send_type_1.TYPE1_5EV;
	data = data |(type1_send_type_1.TYPE1_VCC_12V_EN << 1);
	data = data |(type1_send_type_1.TYPE1_TX2_SATA << 3);
	data = data |(type1_send_type_1.TYPE1_TX2_NO_SATA << 4);
	send_data[6] = data;
	send_data[7] = 0x00;
	crt = send_data[0] ^ send_data[1];
	for(int i = 2; i < 8;i++){
		crt = crt ^ send_data[i];
	}	
	send_data[8] = crt;	
	send_data[9] = 0x0d;
	int rLenNow;
	int sret = uartSendIrq(UART1,(unsigned char *)send_data,sizeof(send_data),&rLenNow);
	if(sret == UART1_SEND_OVER){
			uartSendIrq(UART1,(unsigned char *)send_data,sizeof(send_data),&rLenNow);
	}

}

int receive_zynq_type1()//接收
{
		int power_rLenNow = 0;
	int flag = 0;
	uartReadIrq(UART1,(unsigned char *)power_rdata_type1,sizeof(power_rdata_type1),&power_rLenNow);
		for(int i= 0;i < power_rLenNow;i++){
			if(power_rdata_type1[i] == receive_HADE_0){
				flag = 1;
				type1_data_strat = i;
			}
			else if(power_rdata_type1[i] == receive_TALL){
				flag = 2;
				type1_data_end = i;
				if(power_rdata_type1[type1_data_strat+2] == 0x02){
					type1_send_type0_Ask_power_list();
					save_list_flag = 2;
						memset(power_rdata_type1,0,sizeof(power_rdata_type1));
						uartReadIrqReset(UART1);
				}
				if(power_rdata_type1[type1_data_strat+2] == 0x01){
						Analysis_zynq_type1(0x01);//如果cmd为1
							send_zynq_type1_flag = 2;
						type1mcu_send_switch_status_type0mcu();
							uartReadIrqReset(UART1);
							memset(power_rdata_type1,0,sizeof(power_rdata_type1));
					}
				if(power_rdata_type1[type1_data_strat+2] == 0x03){
						Analysis_zynq_type1(0x01);
							uartReadIrqReset(UART1);
							memset(power_rdata_type1,0,sizeof(power_rdata_type1));
				}
				}
			}
}
int send_FPGA_data_Power_list_type1()//发送电源列表给FGPA
{
        char crt;
	char data_buf[20]={0};
	int b = 0;
	for(int i = type1_data_strat;i <=type1_data_end;i++){
		data_buf[b++] = power_rdata_type1[i];
	}
        crt = data_buf[0] ^ data_buf[1];
	for(int i = 2;i < 5;i++){
		crt = crt ^ data_buf[i];
	}
	if(crt != data_buf[type1_data_end-1]){
		return -1;
	}
	char send_data[15]={0};
	send_data[0] = SEND_HADE_0;
	send_data[1] = SEND_HADE_1;
	send_data[2] = 0x02;
	send_data[3] = send_num_FPGA++;
	send_data[4] = 0x06;
	char data0 = 0x00,data1 = 0x00,data2 = 0x00;
	data0 = data0 | type1_Save_state_type0.TYPE0_ZYNQ;
	data0 = data0 | type1_Save_state_type0.TYPE0_4G << 1;
	data0 = data0 | type1_Save_state_type0.TYPE0_VCC_12V_EN<< 2;
	
	data1 = data1 | type1_Save_state_type1.TYPE1_5EV;
	data1 = data1 | type1_Save_state_type1.TYPE1_TX2_SATA << 1;
	data1 = data1 | type1_Save_state_type1.TYPE1_TX2_NO_SATA  << 2;
	data1 = data1 | type1_Save_state_type1.TYPE1_VCC_12V_EN << 3;
	
	data2 = data2 | type1_Save_state_type2.TYPE2_ZYNQ;
	data2 = data2 | type1_Save_state_type2.TYPE2_VCC_12V_EN << 1;
	
	
	send_data[5] = data0;
	send_data[6] = data1;
	send_data[7] = data2;
	send_data[8] = 0x00;
	send_data[9] = 0x00;
	send_data[10] = 0x01;
	crt = send_data[0] ^ send_data[1];
	for(int i = 2;i < 11;i++){
		crt = crt ^ send_data[i];
	}
	send_data[11] = crt;
	send_data[12] = 0x0d;
	int rLenNow;
	int sret = uartSendIrq(UART1,(unsigned char *)send_data,sizeof(send_data),&rLenNow);
	if(sret == UART1_SEND_OVER){
				uartSendIrq(UART1,(unsigned char *)send_data,sizeof(send_data),&rLenNow);
	}

}




int Analysis_zynq_type1(int cmd)//解析关闭电源是否允许关闭
{
	int crt;
	char data_buf[20]={0};
	int b = 0;
	for(int i = type1_data_strat;i <=type1_data_end;i++){
		data_buf[b++] = power_rdata_type1[i];
	}
        crt = data_buf[0] ^ data_buf[1];
	for(int i = 2;i < type1_data_end-type1_data_strat-1;i++){
		crt = crt ^ data_buf[i];
	}
	if(crt != data_buf[type1_data_end-1]){
		return -1;
	}
	if(cmd == 0x03){
		if(data_buf[5] == ON){
				type1_Save_state_type1.switch_flag = ON;
				if(data_buf[6] & 1){
					type1_Save_state_type1.TYPE1_5EV = 1;
					Execution_TYPE1_5EV(ON);
				}
			 if(data_buf[6]>>1 & 1){
					type1_Save_state_type1.TYPE1_VCC_12V_EN = 1;
					Execution_TYPE1_TYPE1_VCC_12V_EN(ON);
				}
			 if(data_buf[6]  >> 3 & 1){
					type1_Save_state_type1.TYPE1_TX2_SATA = 1;
					Execution_TYPE1_TX2_SATA(ON);
				}
				if(data_buf[6]  >> 4 & 1){
					type1_Save_state_type1.TYPE1_TX2_NO_SATA = 1;
					Execution_TYPE1_TX2_NO_SATA(ON);
				}
					
			}
			else if(data_buf[5] == OFF){
				type1_Save_state_type1.switch_flag = OFF;
				if(data_buf[6] & 1){
					type1_Save_state_type1.TYPE1_5EV = 0;
					Execution_TYPE1_5EV(OFF);
				}
			 if(data_buf[6] >>1 & 1){
					type1_Save_state_type1.TYPE1_VCC_12V_EN = 0;
					Execution_TYPE1_TYPE1_VCC_12V_EN(OFF);
				}
			 if(data_buf[6]  >> 3 & 1){
					type1_Save_state_type1.TYPE1_TX2_SATA = 0;
					Execution_TYPE1_TX2_SATA(OFF);
				}
				if(data_buf[6]  >> 4 & 1){
					type1_Save_state_type1.TYPE1_TX2_NO_SATA = 0;
					Execution_TYPE1_TX2_NO_SATA(OFF);
				}
				
			}
	}
	else if(cmd == 0x01){
		if(data_buf[5] == ON && type1_send_type_1.switch_flag == ON){
				type1_Save_state_type1.switch_flag = ON;
				if((data_buf[6] & 1) && type1_send_type_1.TYPE1_5EV==1){
					type1_Save_state_type1.TYPE1_5EV = 1;
					Execution_TYPE1_5EV(ON);
				}
			 if((data_buf[6]>>1 & 1) && type1_send_type_1.TYPE1_VCC_12V_EN == 1){
					type1_Save_state_type1.TYPE1_VCC_12V_EN = 1;
					Execution_TYPE1_TYPE1_VCC_12V_EN(ON);
				}
			 if((data_buf[6]  >> 3 & 1) && type1_send_type_1.TYPE1_TX2_SATA == 1){
					type1_Save_state_type1.TYPE1_TX2_SATA = 1;
					Execution_TYPE1_TX2_SATA(ON);
				}
				if((data_buf[6]  >> 4 & 1)&& type1_send_type_1.TYPE1_TX2_NO_SATA == 1){
					type1_Save_state_type1.TYPE1_TX2_NO_SATA = 1;
					Execution_TYPE1_TX2_NO_SATA(ON);
				}
					
			}
			else if(data_buf[5] == OFF && type1_send_type_1.switch_flag== OFF){
				type1_Save_state_type1.switch_flag = OFF;
				if((data_buf[6] & 1) && type1_send_type_1.TYPE1_5EV==1){
					type1_Save_state_type1.TYPE1_5EV = 0;
					Execution_TYPE1_5EV(OFF);
				}
			 if((data_buf[6]>>1 & 1) && type1_send_type_1.TYPE1_VCC_12V_EN == 1){
					type1_Save_state_type1.TYPE1_VCC_12V_EN = 0;
					Execution_TYPE1_TYPE1_VCC_12V_EN(OFF);
				}
			 if((data_buf[6]  >> 3 & 1) && type1_send_type_1.TYPE1_TX2_SATA == 1){
					type1_Save_state_type1.TYPE1_TX2_SATA = 0;
					Execution_TYPE1_TX2_SATA(OFF);
				}
				if((data_buf[6]  >> 4 & 1)&& type1_send_type_1.TYPE1_TX2_NO_SATA == 1){
					type1_Save_state_type1.TYPE1_TX2_NO_SATA = 0;
					Execution_TYPE1_TX2_NO_SATA(OFF);
				}
				
			}
	}
			
	
}







