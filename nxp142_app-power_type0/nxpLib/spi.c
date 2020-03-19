#include "device_registers.h" /* include peripheral declarations S32K144 */
#include "stdint.h"
#include "string.h"
#include "usertype.h"
//#include "null.h"
#include "nxpQueue.h"

/*
 * ivcu only spi0 connect out, so only consider spi0
 * below spi all spi0
 * baudrate is 1MHZ
 * */

static queue str_recvQueue;
static queue str_sendQueue;

static int queueMaxSize = 100;
static int spi_sendStatus = 0, spi_readStatus;
static uint32_t spi_sendNum = 0, spi_needSendNum = 0;
static uint32_t spi_readNum = 0, spi_needReadNum = 0;
unsigned char spi_buf[1024] = {0}, spi_recv[1024] = {0};

void SpiInitMaster()
{
	initQueue(&str_recvQueue, queueMaxSize);
	initQueue(&str_sendQueue, queueMaxSize);
	spi_readStatus = SPI_READ_OVER;
	spi_sendStatus = SPI_SEND_OVER;

	PCC->PCCn[PCC_PORTB_INDEX ]|=PCC_PCCn_CGC_MASK; /* Enable clock for PORTB */
	  PORTB->PCR[14]|=PORT_PCR_MUX(3); /* Port B14: MUX = ALT3, LPSPI1_SCK */
	  PORTB->PCR[15]|=PORT_PCR_MUX(3); /* Port B15: MUX = ALT3, LPSPI1_SIN */
	  PORTB->PCR[16]|=PORT_PCR_MUX(3); /* Port B16: MUX = ALT3, LPSPI1_SOUT */
	  PORTB->PCR[17]|=PORT_PCR_MUX(3); /* Port B17: MUX = ALT3, LPSPI1_PCS3 */

	/*!
	 * SPI0 Clocking:
	 * ===================================================
	 */
	PCC->PCCn[PCC_LPSPI1_INDEX] = 0;          		/* Disable clocks to modify PCS ( default) 	*/
	PCC->PCCn[PCC_LPSPI1_INDEX] = PCC_PCCn_PR_MASK	/* (default) Peripheral is present.			*/
								 |PCC_PCCn_CGC_MASK	/* Enable PCS=SPLL_DIV2 (40 MHz func'l clock) 	*/
								 |PCC_PCCn_PCS(6);
	/*!
	 * LPSPI1 Initialization:
	 * ===================================================
	 */
  LPSPI1->CR    = 0x00000000;   			/* Disable module for configuration 			*/
  LPSPI1->IER   =  LPSPI_IER_RDIE_MASK
				  | LPSPI_IER_TEIE_MASK
				  | LPSPI_IER_FCIE_MASK;   			/* Interrupts not used 						*/
//  LPSPI1->IER   = LPSPI_IER_TDIE_MASK;

  LPSPI1->DER   = 0x00000000;   			/* DMA not used 								*/
  LPSPI1->CFGR0 = 0x00000000;   			/* Defaults: 									*/
                                			/* RDM0=0: rec'd data to FIFO as normal 		*/
                                			/* CIRFIFO=0; Circular FIFO is disabled 		*/
                                			/* HRSEL, HRPOL, HREN=0: Host request disabled */

  LPSPI1->CFGR1 = LPSPI_CFGR1_MASTER_MASK  /* Configurations: master mode									*/
				  | LPSPI_CFGR1_NOSTALL_MASK;						/* PCSCFG=0: PCS[3:2] are enabled 								*/
											/* OUTCFG=0: Output data retains last value when CS negated	*/
											/* PINCFG=0: SIN is input, SOUT is output 						*/
											/* MATCFG=0: Match disabled 									*/
											/* PCSPOL=0: PCS is active low 								*/
											/* NOSTALL=0: Stall if Tx FIFO empty or Rx FIFO full 			*/
											/* AUTOPCS=0: does not apply for master mode 					*/
											/* SAMPLE=0: input data sampled on SCK edge 					*/
											/* MASTER=1: Master mode 										*/

  LPSPI1->TCR   = LPSPI_TCR_CPHA_MASK
				  |LPSPI_TCR_PRESCALE(2)
				  |LPSPI_TCR_CONTC(0)  //
				  |LPSPI_TCR_CONT(0)   //
				  |LPSPI_TCR_PCS(3)
				  |LPSPI_TCR_FRAMESZ(31);   /* Transmit cmd: PCS3, 16 bits, prescale func'l clk by 4, etc	*/
											/* CPOL=0: SCK inactive state is low 							*/
											/* CPHA=1: Change data on SCK lead'g, capture on trail'g edge	*/
											/* PRESCALE=2: Functional clock divided by 2**2 = 4 			*/
											/* PCS=3: Transfer using PCS3 									*/
											/* LSBF=0: Data is transfered MSB first 						*/
											/* BYSW=0: Byte swap disabled 									*/
											/* CONT, CONTC=0: Continuous transfer disabled 				*/
											/* RXMSK=0: Normal transfer: rx data stored in rx FIFO 		*/
											/* TXMSK=0: Normal transfer: data loaded from tx FIFO 			*/
											/* WIDTH=0: Single bit transfer 								*/
											/* FRAMESZ=31: # bits in frame = 31+1=32 						*/

  LPSPI1->CCR   = LPSPI_CCR_SCKPCS(4)
				  |LPSPI_CCR_PCSSCK(4)
				  |LPSPI_CCR_DBT(8)
				  |LPSPI_CCR_SCKDIV(8);   	/* Clock dividers based on prescaled func'l clk of 100 nsec 	*/
											/* SCKPCS=4: SCK to PCS delay = 4+1 = 5 (500 nsec) 			*/
											/* PCSSCK=4: PCS to SCK delay = 9+1 = 10 (1 usec) 				*/
											/* DBT=8: Delay between Transfers = 8+2 = 10 (1 usec) 			*/
											/* SCKDIV=8: SCK divider =8+2 = 10 (1 usec: 1 MHz baud rate) 	*/

  LPSPI1->FCR   = LPSPI_FCR_TXWATER(3);   	/* RXWATER=0: Rx flags set when Rx FIFO >0 	*/
                                			/* TXWATER=3: Tx flags set when Tx FIFO <= 3 	*/

  LPSPI1->CR    = LPSPI_CR_MEN_MASK;
		  	  	  //|LPSPI_CR_DBGEN_MASK;   	/* Enable module for operation 			*/
											/* DBGEN=1: module enabled in debug mode 	*/
											/* DOZEN=0: module enabled in Doze mode 	*/
											/* RST=0: Master logic not reset 			*/
											/* MEN=1: Module is enabled 				*/
}

int spiIrqInit(int port){

	if(port != SPI0 && port != SPI1)	return -1;
		S32_NVIC->ICPR[0] = 1 << ((LPSPI0_IRQn + port) % 32);  /* clr any pending IRQ*/
		S32_NVIC->ISER[0] = 1 << ((LPSPI0_IRQn + port) % 32);  /* enable IRQ */
		S32_NVIC->IP[LPSPI0_IRQn + port] = 0xA;              /* priority 10 of 0-15*/

}

uint32_t SpiSend (uint8_t *wData, int32_t len, uint32_t timeOut_ms)
{
	int i = 0;
	uint32_t timeOutTemp = 0, sendNum = 0;
	uint32_t val = 0;
	if(len < 0 || wData == NULL)	return 0;
	timeOut_ms *= 8000;
	if(len >= 4){
		for(i=0; i<len - len%4; i=i+4){
			  while(((LPSPI1->SR & LPSPI_SR_TDF_MASK)>>LPSPI_SR_TDF_SHIFT) == 0){
				  timeOutTemp++;
				  if(timeOutTemp >= timeOut_ms){
					  return sendNum;
				  }
			  }
			  sendNum += 4;
				/* Wait for Tx FIFO available 	*/
			  LPSPI1->TDR = wData[i] | (wData[i+1]<<8) | (wData[i+2] << 16) | (wData[i+3] << 24);              /* Transmit data 				*/
			  LPSPI1->SR |= LPSPI_SR_TDF_MASK; /* Clear TDF flag 				*/
			  if(sendNum == len)	return sendNum;
		}
	}
	while(((LPSPI1->SR & LPSPI_SR_TDF_MASK)>>LPSPI_SR_TDF_SHIFT) == 0){
		  timeOutTemp++;
		  if(timeOutTemp >= timeOut_ms){
			  return sendNum;
		  }
    }
	for(i=len-len%4; i<len; i++){
		val |= wData[i];
	}
	sendNum = len;
	LPSPI1->TDR = val;
	LPSPI1->SR |= LPSPI_SR_TDF_MASK; /* Clear TDF flag 				*/
	return sendNum;
}
/*
 * len % 4 == 0
 * */
uint32_t SpiRecv (uint8_t *rData, int32_t len, uint32_t timeOut_ms)
{
	  uint32_t recieve = 0;
	  uint32_t timeOutTemp = 0, readNum = 0;
	  timeOut_ms *= 8000;

	  while(len > readNum){
		  while((LPSPI1->SR & LPSPI_SR_RDF_MASK)>>LPSPI_SR_RDF_SHIFT==0){
			  timeOutTemp++;
			  if(timeOutTemp >= timeOut_ms){
				  return readNum;
			  }
		  }
		  readNum += 4;
										   /* Wait at least one RxFIFO entry 	*/
		  recieve= LPSPI1->RDR;            /* Read received data 				*/
		  memcpy(rData + readNum, recieve, 4);
		  LPSPI1->SR |= LPSPI_SR_RDF_MASK; /* Clear RDF flag 					*/

		  if(readNum == len)	return readNum;
	  }
	  return readNum;                  /* Return received data 	length		*/
}

int spiReadIrq(unsigned char *rData, int32_t len)
{
	int ret = -1;
	if(len > queueMaxSize)	return ret;

	if(spi_needReadNum == 0){
		spi_needReadNum = len;
		spi_readStatus = SPI_READ_ING;
	}
	else if(spi_readNum == spi_needReadNum){
		memcpy(rData, spi_recv, len);
		spi_readNum = spi_needReadNum = 0;
		spi_readStatus = SPI_READ_OVER;
	}
	ret = spi_readStatus;

	return ret;
}
int SpiSendIrq (uint8_t *wData, int32_t len)
{
	int ret = -1, i = 0;
	ElemType  _elemIn;
	if(len > queueMaxSize)	return ret;
	if(spi_needSendNum == 0){
		spi_needSendNum = len;
		spi_sendStatus = SPI_SEND_ING;
		for(i=0; i<len; i++){
			_elemIn.data = wData[i];
			enQueue(&str_sendQueue, _elemIn);
		}
		LPSPI1->IER |= LPSPI_IER_TDIE_MASK;
	}
	else if(spi_needSendNum == spi_sendNum){
		spi_sendNum = spi_needSendNum = 0;
		spi_sendStatus = SPI_SEND_OVER;
	}
	ret = spi_sendStatus;
	return ret;
}

void LPSPI1_IRQHandler(uint32_t instance)
{
	uint32_t recvVal = 0, sendVal = 0;
	uint16_t txCount, rxCount, sendFlag = 0;
	ElemType _elemType, _elemOut, _elemIn;
	int i = 0;
//	txCount = LPSPI1->FSR & 0x07;
	rxCount = (LPSPI1->FSR >> 16) & 0x07;

	/* RECEIVE IRQ handler: Check read buffer only if there are remaining bytes to read. */
	if((LPSPI1->SR & LPSPI_SR_RDF_MASK)>>LPSPI_SR_RDF_SHIFT==1 && (rxCount != (uint16_t)0) && (LPSPI1->SR & LPSPI_SR_REF_MASK)>>LPSPI_SR_RDF_SHIFT==1 )
	{
		 recvVal = LPSPI1->RDR;
		for(i = 0;i< rxCount; i++){
			_elemType.data = (recvVal >> (8*i)) & 0xff;
			enQueue(&str_recvQueue,_elemType);
		}
		if(spi_needReadNum > 0){
			while(getQueueLen(&str_recvQueue) > 0 && spi_readNum<spi_needReadNum){
				deQueue(&str_recvQueue, &_elemOut);
				spi_recv[spi_readNum++] = _elemOut.data;
			}
		}
	}
	/* Transmit data */
	if ((LPSPI1->SR & LPSPI_SR_TDF_MASK)>>LPSPI_SR_TDF_SHIFT==1 /*&& (txCount != (uint16_t)0)*/)
	{
		for(i=0; i< 4; i++){
			if(getQueueLen(&str_sendQueue) > 0){
				deQueue(&str_sendQueue, &_elemIn);
				sendVal |=  ((uint32_t)_elemIn.data << (8*i));
				spi_sendNum++;
				sendFlag = 1;
			}
		}
		if(sendFlag){
			LPSPI1->TDR = sendVal;
			LPSPI1->SR |= LPSPI_SR_TDF_MASK; /* Clear TDF flag 				*/
		}
		if(getQueueLen(&str_sendQueue) <= 0){
			LPSPI1->IER &= (~LPSPI_IER_TDIE_MASK);
		}
	}
}

