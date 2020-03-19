#ifndef TYPE0_MCU_TEST_H_
#define TYPE0_MCU_TEST_H_


int type0_MCU_test_init();
int MCU_test_Analysis(int rdatalen);
int type0_MCU_test_Canbus();
int type0_MCU_test_Send(int id,int len_data,unsigned char *data);
int type0_MCU_test_LTE();
void type0_MCU_Automatic_testing_Start(char *rdata,int rLenNow);
void type0_MCU_test_Power_Management();
int type0_MCU_test_WDOG();
extern int LTE_status;
void delay1(volatile int cycles);
#endif