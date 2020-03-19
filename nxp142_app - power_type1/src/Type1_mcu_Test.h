#ifndef TYPE1_MCU_TEST_H_
#define TYPE1_MCU_TEST_H_


int TYPE1_MCU_test_init();
int TYPE1_MCU_test_Analysis();
int TYPE1_MCU_test_Canbus();
int TYPE1_MCU_test_Send(int id,int len_data,unsigned char *data);
int TYPE1_MCU_test_LTE();
void TYPE1_MCU_Automatic_testing_Start();
void TYPE1_MCU_test_Power_Management();
int TYPE1_MCU_test_WDOG();
void delay1(volatile int cycles);
#endif