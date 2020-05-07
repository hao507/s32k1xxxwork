#ifndef NXPLIB_CANBUS_H_
#define NXPLIB_CANBUS_H_
#include  <stdint.h>
#include "usertype.h"

#define CANBUS0 0
#define CANBUS1 1
int CanbusInit(uint8_t canport,uint32_t baudrate);
void CanbusIrqInit(uint8_t canport);
int CanbusSend(uint8_t canport,uint32_t id, uint8_t *wdata, uint8_t len, uint8_t mode);
void CanbusRecv(uint8_t canport,uint32_t *id, uint8_t *rdata, uint8_t *len);
int canbusReadIrq(uint8_t canport,int *id, uint8_t *rData, int32_t *queueLen);


int Canbus1Init(uint32_t baudrate);
void Canbus1IrqInit();
int Canbus1Send(uint32_t id, uint8_t *wdata, uint8_t len, uint8_t mode);
void Canbus1Recv(uint32_t *id, uint8_t *rdata, uint8_t *len);
int canbus1ReadIrq(int *id, uint8_t *rData, int32_t *queueLen);
void Canbus_irq_Reset(uint8_t canport);
void Canbus0_irq_Reset();
void disable_can1();
void disable_can(uint8_t canport);
void canbus1buffrest();
void canbus0buffrest();
#endif