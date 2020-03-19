/*
 * uart.c
 *
 *  Created on: 2019��5��14��
 *      Author: Administrator
 */
#include "device_registers.h" /* include peripheral declarations S32K144 */
#include "stdint.h"
#include "usertype.h"
#include "stdlib.h"
#include "string.h"
#include "nxpQueue.h"

static unsigned char uartIrq = 0;

static uint8_t	uart0_readStatus = 0, uart0_sendStatus = 0;
static uint32_t uart0_readNum = 0, uart0_needReadNum = 0;
static uint32_t uart0_sendNum = 0, uart0_needSendNum = 0;
unsigned char uart0_buf[1024] = {0}, uart0_recv[1024] = {0};

static uint8_t	uart1_readStatus = 0, uart1_sendStatus = 0;
static uint32_t uart1_readNum = 0, uart1_needReadNum = 0;
static uint32_t uart1_sendNum = 0, uart1_needSendNum = 0;
unsigned char uart1_buf[1024] = {0}, uart1_recv[1024] = {0};

static queue str_recvQueue0;
static queue str_recvQueue1;

static queue str_sendQueue0;
static queue str_sendQueue1;

static int queueMaxSize = 200;

//---------------------------------------------------------------------------
int uartInit(uint8_t port, uint32_t baudrate)
{
	int i = 0, sbr = 0, osr = 0;
	int speed_table[9]={4800, 9600, 19200, 38400, 57600, 115200, 230400,460800};
	if(port != UART1 && port != UART0){
//		printf("uart port err: %d...\n", port);
		return -1;
	}
	if(port == UART0){
			initQueue(&str_recvQueue0, queueMaxSize);
			initQueue(&str_sendQueue0, queueMaxSize);
	}
	else{
		initQueue(&str_recvQueue1, queueMaxSize);
		initQueue(&str_sendQueue1, queueMaxSize);
	}
	uart0_readStatus = UART0_READ_OVER;
	uart1_readStatus = UART1_READ_OVER;
	uart0_sendStatus = UART0_SEND_OVER;
	uart1_sendStatus = UART1_SEND_OVER;
	for(i=0; i< 9; i++){
		if(baudrate == speed_table[i]){
			switch(i){
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
			case 7:
				osr = 15;
				sbr = (int)((8000000.0 / baudrate) / (osr + 1) + 0.5);
				break;
			case 5:
				osr = 4;
				sbr = 14;
				break;
			case 6:
				osr = 1;
				sbr = 23;
				break;
			default:
				break;
			}
			break;
		}
	}
	if(i == 12){
//		printf("uart baudrate err: %d...\n", baudrate);
		return -2;
	}
	if(port == UART0){
		PCC->PCCn[PCC_PORTA_INDEX ]|=PCC_PCCn_CGC_MASK; /* Enable clock for PORTC */
		PORTA->PCR[2]|=PORT_PCR_MUX(6);
		PORTA->PCR[3]|=PORT_PCR_MUX(6);
	}
	else if(port == UART1){
		PCC->PCCn[PCC_PORTC_INDEX ]|=PCC_PCCn_CGC_MASK; /* Enable clock for PORTC */
		PORTC->PCR[8]|=PORT_PCR_MUX(2);	/* Port C8: MUX = ALT2, UART1 RX */
		PORTC->PCR[9]|=PORT_PCR_MUX(2);   /* Port C9: MUX = ALT2, UART1 TX */
	}

	PCC->PCCn[PCC_LPUART0_INDEX + port] &= ~PCC_PCCn_CGC_MASK;    /* Ensure clk disabled for config */
	PCC->PCCn[PCC_LPUART0_INDEX + port] |= PCC_PCCn_PCS(0X02)    /* Clock Src = 2 (SIRCDIV2_CLK) 0b010*/
								 |  PCC_PCCn_CGC_MASK;     /* Enable clock for LPUART1 regs */

	if(port == UART0){
		LPUART0->BAUD = LPUART_BAUD_SBR(sbr)
						|LPUART_BAUD_OSR(osr);

		LPUART0->CTRL =	LPUART_CTRL_RE_MASK
						|LPUART_CTRL_TE_MASK;
	}
	else if(port == UART1){
		LPUART1->BAUD = LPUART_BAUD_SBR(sbr)  	/* Initialize for 9600 baud, 1 stop: */
								|LPUART_BAUD_OSR(osr);  	/* SBR=52 (0x34): baud divisor = 8M/9600/16 = ~52 */
														/* OSR=15: Over sampling ratio = 15+1=16 */
														/* SBNS=0: One stop bit */
														/* BOTHEDGE=0: receiver samples only on rising edge */
														/* M10=0: Rx and Tx use 7 to 9 bit data characters */
														/* RESYNCDIS=0: Resync during rec'd data word supported */
														/* LBKDIE, RXEDGIE=0: interrupts disable */
														/* TDMAE, RDMAE, TDMAE=0: DMA requests disabled */
														/* MAEN1, MAEN2,  MATCFG=0: Match disabled */

		LPUART1->CTRL =	LPUART_CTRL_RE_MASK
						|LPUART_CTRL_TE_MASK;	
//						|LPUART_CTRL_RIE_MASK;
//						|LPUART_CTRL_TIE_MASK
//						|LPUART_CTRL_TCIE_MASK;

	}

	return 0;
}

int uartIrqInit(uint8_t port)
{
	if(port != UART0 && port != UART1)	return -1;
	if(port == UART0)
		LPUART0->CTRL |=	LPUART_CTRL_RIE_MASK;
	else if(port == UART1)
		LPUART1->CTRL |=	LPUART_CTRL_RIE_MASK;
	S32_NVIC->ICPR[(LPUART0_RxTx_IRQn + port*2) >> 5] = 1 << ((LPUART0_RxTx_IRQn + port*2) % 32);  /* clr any pending IRQ*/
	S32_NVIC->ISER[(LPUART0_RxTx_IRQn + port*2) >> 5] = 1 << ((LPUART0_RxTx_IRQn + port*2) % 32);  /* enable IRQ */
	S32_NVIC->IP[LPUART0_RxTx_IRQn + port*2] = 0xA;              /* priority 10 of 0-15*/
	uartIrq |= port;
	return 0;
}

void UART_Reset(int port)
{
	/* Set to after reset state an disable UART Tx/Rx */
	if(port == 0){
		LPUART0->CTRL = 0x00000000;
		LPUART0->BAUD = 0x00000000;
	}
	else if(port == 1){
		LPUART1->CTRL = 0x00000000;
		LPUART1->BAUD = 0x00000000;
	}
	S32_NVIC->ICER[(LPUART0_RxTx_IRQn + port*2) >> 5] = 1 << ((LPUART0_RxTx_IRQn + port*2) % 32);  /* disable IRQ */
	
	/* Disable clock to UART */
	PCC->PCCn[PCC_LPUART0_INDEX] &= ~PCC_PCCn_CGC_MASK;     /* Enable clock for LPUART0 regs */
	PCC->PCCn[PCC_LPUART1_INDEX] &= ~PCC_PCCn_CGC_MASK;     /* Enable clock for LPUART1 regs */

}

void UART_pin_settings_reset(int port){
	if(port == UART0){
		PCC->PCCn[PCC_PORTA_INDEX ]|=PCC_PCCn_CGC_MASK; /* Enable clock for PORTC */
		PORTA->PCR[2]|=PORT_PCR_MUX(1);
		PORTA->PCR[3]|=PORT_PCR_MUX(1);
	}
	else if(port == UART1){
		PCC->PCCn[PCC_PORTC_INDEX ]|=PCC_PCCn_CGC_MASK; /* Enable clock for PORTC */
		PORTC->PCR[8]|=PORT_PCR_MUX(1);	/* Port C8: MUX = ALT2, UART1 RX */
		PORTC->PCR[9]|=PORT_PCR_MUX(1);   /* Port C9: MUX = ALT2, UART1 TX */
	}
}
//----------------------------------------------------------------------------------------
int uartSendBlock(uint8_t port, unsigned char *wData, int32_t len, uint32_t timeOut)
{
	int timeOutTemp = 0, sendNum = 0;
	timeOut *= 8000;
	while(len--){
		if(port == UART0){
			while(((LPUART0->STAT & LPUART_STAT_TDRE_MASK)>>LPUART_STAT_TDRE_SHIFT==0)){/* Wait for transmit buffer to be empty */
				timeOutTemp++;
				if(timeOutTemp >= timeOut)	return sendNum;
			}
				LPUART0->DATA=wData[sendNum++];
		}
		else if(port == UART1){
			while(((LPUART1->STAT & LPUART_STAT_TDRE_MASK)>>LPUART_STAT_TDRE_SHIFT==0)){/* Wait for transmit buffer to be empty */
				timeOutTemp++;
				if(timeOutTemp >= timeOut)	return sendNum;
			}
				LPUART1->DATA=wData[sendNum++];
		}
	}
	return sendNum;
}

int uartReadBlock(uint8_t port, unsigned char *rData, int32_t len, uint32_t timeOut)
{
	int timeOutTemp = 0, recvNum = 0;
	char valTemp = 0;
	timeOut *= 8000;
	if(len > 0){
		while(len--){
			if(port == UART0){
				//OR
				if (LPUART0->STAT & 0x00080000)
						LPUART0->STAT |= 0x00080000;
				while((LPUART0->STAT & LPUART_STAT_RDRF_MASK)>>LPUART_STAT_RDRF_SHIFT==0)
					{/* Wait for received buffer to be full */
					timeOutTemp++;
					if(timeOutTemp >= timeOut)	return recvNum;
				}
				valTemp= LPUART0->DATA;            /* Read received data*/
				rData[recvNum++] = valTemp;
			}
			else if(port == UART1){
				//OR
				if (LPUART1->STAT & 0x00080000)
						LPUART1->STAT |= 0x00080000;
				while((LPUART1->STAT & LPUART_STAT_RDRF_MASK)>>LPUART_STAT_RDRF_SHIFT==0){/* Wait for received buffer to be full */
					timeOutTemp++;
					if(timeOutTemp >= timeOut)	return recvNum;
				}
				valTemp= LPUART1->DATA;            /* Read received data*/
				rData[recvNum++] = valTemp;
			}
			}

	}
	return recvNum;
}

//------------------------------------------------------------------
int uartReadIrq(uint8_t port,  unsigned char *rData, int32_t len, int32_t* rLenNow)
{
	int ret = -1;
	if(len > queueMaxSize)	return ret;

	if(UART0 == port){
		//OR
		if (LPUART0->STAT & 0x00080000)
			LPUART0->STAT |= 0x00080000;
	
		if(uart0_needReadNum == 0){
			uart0_needReadNum = len;
			uart0_readStatus = UART0_READ_ING;
		}
		else if(uart0_readNum == uart0_needReadNum){
			memcpy(rData, uart0_recv, len);
    		uart0_readNum = uart0_needReadNum = 0;
    		uart0_readStatus = UART0_READ_OVER;
    	}
		else if(uart0_readNum > 0){
			memcpy(rData, uart0_recv, uart0_readNum);
			*rLenNow = uart0_readNum;
		}
		ret = uart0_readStatus;
	}
	else if(UART1 == port){
		//OR
		if (LPUART1->STAT & 0x00080000)
			LPUART1->STAT |= 0x00080000;
		
		if(uart1_needReadNum == 0){
			uart1_needReadNum = len;
			uart1_readStatus = UART1_READ_ING;
		}
		else if(uart1_readNum == uart1_needReadNum){
			memcpy(rData, uart1_recv, len);
			uart1_readNum = uart1_needReadNum = 0;
			uart1_readStatus = UART1_READ_OVER;
		}
		else if(uart1_readNum > 0){
			memcpy(rData, uart1_recv, uart1_readNum);
			*rLenNow = uart1_readNum;
		}
		ret = uart1_readStatus;
	}
	return ret;
}

