#ifndef UPDATA_H_
#define UPDATA_H_
#include "stdint.h"
extern int8_t updata_status;

#define MCURECV_HEAD			0XAA55
#define CMD_HANDSHAKE			0X01
#define CMD_ASK_UPDATA		0X02
#define CMD_UPDATA				0X03
#define CMD_CHECK					0X04
#define CMD_UPDATA_END		0X05
#define CMD_JUMP_BOOT			0X67
#define MCURECV_END				0X0D


#define MCUSEND_HEAD				0X55AA
#define CMD_FB_HANDSHAKE		0X01
#define CMD_FB_ASK_UPDATA		0X02
#define CMD_FB_UPDATA				0X03
#define CMD_FB_CHECK				0X04
#define CMD_FB_JUMP_BOOT		0X67
#define MCUSEND_END					0X0D

enum{
	updata_wait,
	updata_start,
	updata_ing = 3,
	updata_check,
	updata_end,
	updata_jump = 0x67
};

int flash_init(void);
void EraseAndProgram(void);
void flash_updata(void);
void updata_recv(void);
void updata_erase(void);
void updata_handle(uint8_t status, uint8_t *rData, int len, int timeNum);


#endif
