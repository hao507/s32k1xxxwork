#include "updata.h"
#include "flash_driver.h"
#include "pcc_hal.h"
#include "uart.h"
#include "usertype.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "../equipLib/wdgTps.h"
#include "gpio.h"
flash_ssd_config_t flashSSDConfig;
int8_t updata_status = -1;
static uint32_t updataAddrBase = 0x4000, updataAddrOffset = 0;
static uint8_t	crc = 0;
//-------------------------------------------------------------
unsigned char recvData[1024] = {0}, cmdType = 0;
int recvIndex = 0, cmdLen = 0, timeNumRecv = 0, timeNumRecvOld = 0, timeNumSend = 0;
int slenNow = 0;

const flash_user_config_t Flash_InitConfig0 = {
    .PFlashBase  = 0x00000000U,
    .PFlashSize  = 0x00040000U,
    .DFlashBase  = 0x10000000U,
    .EERAMBase   = 0x14000000U,//is flex ram,not the system ram
    /* If using callback, any code reachable from this function must not be placed in a Flash block targeted for a program/erase operation.*/
    .CallBack    = NULL_CALLBACK
};
typedef enum _boolean
{
    FALSE,
    TRUE = !FALSE
} boolean;

uint8_t crc_check(uint8_t *pData, int len)
{
		if(pData == NULL || len <=0)	return -1;
		for(int i=0; i<len; i++){
			crc ^= pData[i];
		}
		return 0;
}

int flash_init(void)
{
		PCC_HAL_SetClockMode(PCC, PCC_FTFC0_CLOCK , TRUE);
    int result = FLASH_DRV_Init(&Flash_InitConfig0, &flashSSDConfig);
    if (result != STATUS_SUCCESS)
    {
        return -1;
    }
		return 0;
}
//__attribute__ ((section("RAMCODE"))); 
void EraseAndProgram(void)
{
	//erase 4KB flash sector (the smallest entity that can be erased) at 0x0004_0000
	while((FTFx_FSTAT & FTFx_FSTAT_CCIF_MASK) == 0);	//wait if operation in progress
	FTFx_FSTAT = FTFx_FSTAT_ACCERR_MASK | FTFx_FSTAT_FPVIOL_MASK;	//clear flags if set

	FTFx_FCCOB0 = 0x09;	//Erase Flash Sector command (0x09)
	FTFx_FCCOB1 = 0x03;	//Flash address [23:16]
	FTFx_FCCOB2 = 0x00;	//Flash address [15:08]
	FTFx_FCCOB3 = 0x00;	//Flash address [7:0]

	FTFx_FSTAT = FTFx_FSTAT_CCIF_MASK;	//launch command
	while((FTFx_FSTAT & FTFx_FSTAT_CCIF_MASK) == 0);	//wait for done


	//program phrase at address 0x0004_0000
	while((FTFx_FSTAT & FTFx_FSTAT_CCIF_MASK) == 0);	//wait if operation in progress
	FTFx_FSTAT = FTFx_FSTAT_ACCERR_MASK | FTFx_FSTAT_FPVIOL_MASK;

	FTFx_FCCOB0 = 0x07;	//Program Phrase command (0x07)
	FTFx_FCCOB1 = 0x03;	//Flash address [23:16]
	FTFx_FCCOB2 = 0x00;	//Flash address [15:08]
	FTFx_FCCOB3 = 0x00;	//Flash address [7:0]

	FTFx_FCCOB4 = 0xFE;	//data
	FTFx_FCCOB5 = 0xED;
	FTFx_FCCOB6 = 0xFA;
	FTFx_FCCOB7 = 0xCE;
	FTFx_FCCOB8 = 0xCA;
	FTFx_FCCOB9 = 0xFE;
	FTFx_FCCOBA = 0xBE;
	FTFx_FCCOBB = 0xEF;

	FTFx_FSTAT = FTFx_FSTAT_CCIF_MASK;	//launch command
	while((FTFx_FSTAT & FTFx_FSTAT_CCIF_MASK) == 0);	//wait for done
}

#pragma arm section code = "RAMCODE"
void updata_erase(void){
	int addr = 0, i = 0;
	unsigned char wData[64] = {0xaa, 0x55, 0x00};
	int index = 2;
	int Num = (Flash_InitConfig0.PFlashSize - 0x4000 -0x20000) / 0x800;
	
	for(i=0; i< Num; i+=1){
		int timeOut = 0;
		
		while((FTFx_FSTAT & FTFx_FSTAT_CCIF_MASK) == 0);//wait if operation in progress
//		{	
//			 if(timeOut++ >= 10){
//				wdgTps_noIrq_process1();
//				 timeOut = 0;
//			 }
//		}
		wdgTps_noIrq_process1();
		timeOut= 0;
		FTFx_FSTAT = FTFx_FSTAT_ACCERR_MASK | FTFx_FSTAT_FPVIOL_MASK;	//clear flags if set
		addr = Flash_InitConfig0.PFlashBase + 0X4000 + i* 0X800;
		
		FTFx_FCCOB0 = 0x09;	//Erase Flash Sector command (0x09)
		FTFx_FCCOB1 = (addr >> 16) & 0X03;	//Flash address [23:16]
		FTFx_FCCOB2 = (addr >> 8) & 0XFF;	//Flash address [15:08]
		FTFx_FCCOB3 = 0x00;	//Flash address [7:0]
		
		addr = (FTFx_FCCOB1<<16) | (FTFx_FCCOB2<<8) | FTFx_FCCOB3;
		if(((FTFx_FCCOB1<<16) | (FTFx_FCCOB2<<8) | FTFx_FCCOB3) < 0x4000)	
		{
			//exit(0);
			break;
		}

		FTFx_FSTAT = FTFx_FSTAT_CCIF_MASK;	//launch command
		//GpioSetVal(PORT_E, 10, PORT_HIGH);
		while((FTFx_FSTAT & FTFx_FSTAT_CCIF_MASK) == 0);//wait for done
//		{	
//			 if(timeOut++ >= 10){
//				 wdgTps_noIrq_process1();
//				 timeOut = 0;
//			 }
//		}
		//GpioSetVal(PORT_E, 10, PORT_LOW);
		wdgTps_noIrq_process1();
		timeOut =0;
		wData[index++] = CMD_FB_ASK_UPDATA;
		wData[index++] = 1;
		wData[index++] = (unsigned char)((float)i/(float)(Num-1) * 100);
		wData[index++] = timeNumSend;
		wData[index++] = MCUSEND_END;
		timeNumSend = (timeNumSend+1 > 0xff) ? 0 : (timeNumSend+1);
		if(wData[4] != 100)
#if TYPE == 0
			uartSendBlock(UART0, wData, index, 10);
#elif TYPE == 1 || TYPE == 2 || TYPE == 3
			uartSendBlock(UART1, wData, index, 10);
#endif
		index = 2;
	}
}

int updata_program(uint8_t *pData, int len){
	if(len > 0 && len%8 != 0 && pData != NULL)
		return -1;
	int i=0, addr = 0;
	for(i=0; i<len; i+=8){
		int timeOut = 0;
		while((FTFx_FSTAT & FTFx_FSTAT_CCIF_MASK) == 0);	//wait if operation in progress
//		{	
//			 if(timeOut++ >= 10){
//				wdgTps_noIrq_process1();
//				 timeOut = 0;
//			 }
//		}
		wdgTps_noIrq_process1();
		FTFx_FSTAT = FTFx_FSTAT_ACCERR_MASK | FTFx_FSTAT_FPVIOL_MASK;
		uint32_t addr = updataAddrBase + updataAddrOffset;
		
		FTFx_FCCOB0 = 0x07;	//Program Phrase command (0x07)
		FTFx_FCCOB1 = (addr>>16) & 0x03;	//Flash address [23:16]
		FTFx_FCCOB2 = (addr>>8) & 0xff;	//Flash address [15:08]
		FTFx_FCCOB3 = addr & 0xff;	//Flash address [7:0]
		addr = (FTFx_FCCOB1<<16) | (FTFx_FCCOB2<<8) | FTFx_FCCOB3;
		
		if(((FTFx_FCCOB1<<16) | (FTFx_FCCOB2<<8) | FTFx_FCCOB3) < 0x4000)	
		{
			exit(0);
			break;
		}

		FTFx_FCCOB7 = pData[i];	//data
		FTFx_FCCOB6 = pData[i+1];
		FTFx_FCCOB5 = pData[i+2];
		FTFx_FCCOB4 = pData[i+3];
		
		FTFx_FCCOBB = pData[i+4];
		FTFx_FCCOBA = pData[i+5];
		FTFx_FCCOB9 = pData[i+6];
		FTFx_FCCOB8 = pData[i+7];

		FTFx_FSTAT = FTFx_FSTAT_CCIF_MASK;	//launch command
		while((FTFx_FSTAT & FTFx_FSTAT_CCIF_MASK) == 0);	//wait for done
//		{	
//			 if(timeOut++ >= 10){
//				wdgTps_noIrq_process1();
//				 timeOut = 0;
//			 }
//		}
		updataAddrOffset += 8;
	}
	crc_check(pData, len);
	
	return 0;
}
#pragma arm section

void flash_updata(void)
{
	wdgTps_noIrq_process1();
	#define READ_ADDR 0X1000
	uint8_t rdata[16] = {0x01, 0x02, 0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e};
	uint32_t val = 0;
	val = *(uint32_t *)READ_ADDR;
	memcpy(rdata, &val, 4);
//	FLASH_DRV_ReadOnce(&flashSSDConfig,0x20,rdata);
	rdata[0] = rdata[0];
	FLASH_DRV_EraseSector(&flashSSDConfig, 0x5000, 0x1000);
	FLASH_DRV_Program(&flashSSDConfig, 0x5000, 8, rdata);
	EraseAndProgram();
	
}

static int eraseSlider = 0;
void updata_handle(uint8_t status, uint8_t *rData, int len, int timeNum)
{
	unsigned char wData[64] = {0xaa, 0x55, 0x00};
	int index = 2, ret = 0;
	switch(status){
		case updata_wait:
			eraseSlider = 0;
			wData[index++] = CMD_FB_HANDSHAKE;
			wData[index++] = 0;
			wData[index++] = timeNumSend;
		  wData[index++] = MCUSEND_END;
		  timeNumSend = (timeNumSend+1 > 0xff) ? 0 : (timeNumSend+1);
			//uartSendIrq(UART0, wData, index, &slenNow);
#if TYPE == 0
			uartSendBlock(UART0, wData, index, 10);
#elif TYPE == 1 || TYPE == 2 || TYPE == 3
			uartSendBlock(UART1, wData, index, 10);
#endif
			break;
		case updata_start:
			if(eraseSlider == 0){
				updata_erase();
				wData[index++] = CMD_FB_ASK_UPDATA;
				wData[index++] = 1;
				wData[index++] = 100;
				wData[index++] = timeNumSend;
				wData[index++] = MCUSEND_END;
				timeNumSend = (timeNumSend+1 > 0xff) ? 0 : (timeNumSend+1);
				eraseSlider = 100;
				//uartSendIrq(UART0, wData, index, &slenNow);
#if TYPE == 0
			uartSendBlock(UART0, wData, index, 10);
#elif TYPE == 1 || TYPE == 2 || TYPE == 3
			uartSendBlock(UART1, wData, index, 10);
#endif
			}
			break;
		case updata_ing:
			if(timeNumRecvOld != timeNum){
					ret = updata_program(rData, len);
			}
			timeNumRecvOld = timeNum;
			wData[index++] = CMD_FB_UPDATA;
			wData[index++] = 1;
		  wData[index++] = (ret >= 0) ? 1 : 0;
			wData[index++] = timeNumSend;
		  wData[index++] = MCUSEND_END;
			timeNumSend = (timeNumSend+1 > 0xff) ? 0 : (timeNumSend+1);
			//uartSendIrq(UART0, wData, index, &slenNow);
#if TYPE == 0
			uartSendBlock(UART0, wData, index, 10);
#elif TYPE == 1 || TYPE == 2 || TYPE == 3
			uartSendBlock(UART1, wData, index, 10);
#endif
			break;
		case updata_check:
			wData[index++] = CMD_FB_CHECK;
			wData[index++] = 1;
		  wData[index++] = crc;
			wData[index++] = timeNumSend;
		  wData[index++] = MCUSEND_END;
			timeNumSend = (timeNumSend+1 > 0xff) ? 0 : (timeNumSend+1);
			//uartSendIrq(UART0, wData, index, &slenNow);
#if TYPE == 0
			uartSendBlock(UART0, wData, index, 10);
#elif TYPE == 1 || TYPE == 2 || TYPE == 3
			uartSendBlock(UART1, wData, index, 10);
#endif
			break;
		case updata_end:
			updata_status = updata_end;
			break;
		case updata_jump:
			wData[index++] = CMD_FB_JUMP_BOOT;
			wData[index++] = 0;
			wData[index++] = timeNumSend;
		  wData[index++] = MCUSEND_END;
		  timeNumSend = (timeNumSend+1 > 0xff) ? 0 : (timeNumSend+1);
			//uartSendIrq(UART0, wData, index, &slenNow);
#if TYPE == 0
			uartSendBlock(UART0, wData, index, 10);
#elif TYPE == 1 || TYPE == 2 || TYPE == 3
			uartSendBlock(UART1, wData, index, 10);
#endif
			break;
		default:
			break;
	}

}

void updata_recv(void)
{
	int i = 0;
	unsigned char rData[120] = {0};
#if TYPE == 0
	int ret = uartReadBlock(UART0,rData, 100,10);
#elif TYPE == 1 || TYPE == 2 || TYPE == 3
	int ret = uartReadBlock(UART1,rData, 100,10);
#endif

	if(ret > 0){
		for(i=0; i<ret; i++){
			if(rData[i] == (MCURECV_HEAD&0xff) && (recvIndex == 0)){
				recvData[recvIndex++] = rData[i];
			}
			else if(rData[i] == ((MCURECV_HEAD >> 8)&0xff) && (recvIndex == 1)){
				recvData[recvIndex++] = rData[i];
			}
			else if(recvIndex == 2){
				cmdType = rData[i];
				recvData[recvIndex++] = cmdType;
			}
			else if(recvIndex == 3){
				cmdLen = rData[i];
				recvData[recvIndex++] = cmdLen;
			}
			else if(recvIndex>=4 && recvIndex<(4+cmdLen)){
				recvData[recvIndex++] = rData[i];
			}
			else if(recvIndex==4+cmdLen){
				timeNumRecv = recvData[recvIndex++] = rData[i];
			}
			else if(rData[i] == MCURECV_END && recvIndex==(5+cmdLen)){
				recvData[recvIndex++] = rData[i];
				if(cmdType == CMD_HANDSHAKE)	
					updata_status = updata_wait;
				else if(cmdType == CMD_ASK_UPDATA)	
					updata_status = updata_start;
				else if(cmdType == CMD_UPDATA)	
					updata_status = updata_ing;
				else if(cmdType == CMD_CHECK)	
					updata_status = updata_check;
				else if(cmdType == CMD_UPDATA_END)	
					updata_status = updata_end;
				wdgTps_noIrq_process1();
				updata_handle(updata_status, recvData+4, cmdLen, timeNumRecv);
				memset(recvData, 0, sizeof(recvData));
				recvIndex = 0;
			}
			else{
				memset(recvData, 0, sizeof(recvData));
				recvIndex = 0;
			}
		}
	}

}


