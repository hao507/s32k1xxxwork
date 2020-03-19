#ifndef TYPE1_MCU_TEST_H_
#define TYPE1_MCU_TEST_H_


int TYPE1_MCU_test_init();
int TYPE1_MCU_test_Analysis();
int TYPE1_MCU_test_Canbus();
int TYPE1_MCU_test_Send(int id,int len_data,unsigned char *data);
int TYPE1_MCU_test_LTE();
int TYPE1_MCU_test_Analysis(int rdatalen);
void TYPE1_MCU_test_Power_Management();
void TYPE1_MCU_Automatic_testing_Start(char *rData,int rLenNow);
int TYPE1_MCU_test_WDOG();
extern int TYPE1_LTE_status;
void delay1(volatile int cycles);
#endif