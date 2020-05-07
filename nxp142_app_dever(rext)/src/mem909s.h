#ifndef MEM909S_H_
#define MEM909S_H_

enum{
	msmSendNo,
	msmSendStep1S,
	msmSendStep1E,
	msmSendStep2S = 3,
	msmSendStep2E,
	msmSendStep3S,
	msmSendFail,
	msmSendOk
};
enum{
	msmReadNo,
	msmReadPrepare,
	msmReadStart,
	msmReadFail,
	msmReadOk
};

int mem909s_msmInit(void);
//read and send can not in the same time
int mem909s_msmRead(char *Phone_number);
int mem909s_msmSend(char *sendData , int len, int firstFlag);
int mem909sReadyGet(void);
int memReadStatusGet(void);
int memSendStatusGet(void);
int ceshi(unsigned char* sendData1, unsigned char* sendData2, int len);
int Low_Power_Consumption();
int mem909s_Init();
#endif
