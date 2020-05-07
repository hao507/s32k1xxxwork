#ifndef TYPE2_MCU_TEST_H_
#define TYPE2_MCU_TEST_H_


int TYPE2_MCU_test_init();
int TYPE2_MCU_test_Analysis();
int TYPE2_MCU_test_Canbus();
int TYPE2_MCU_test_Send(int id,int len_data,unsigned char *data);
int TYPE2_MCU_test_LTE();
int TYPE2_MCU_test_Analysis(int rdatalen);
void TYPE2_MCU_test_Power_Management();
void TYPE2_MCU_Automatic_testing_Start(char *rdata ,int rLenNow);
int TYPE2_MCU_test_WDOG();
extern int TYPE2_LTE_status;

#endif