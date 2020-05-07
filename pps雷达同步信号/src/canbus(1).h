#ifndef NXPLIB_CANBUS_H_
#define NXPLIB_CANBUS_H_
#include  <stdint.h>
#include "usertype.h"
int Canbus0Init(uint32_t baudrate);
void Canbus0IrqInit();
int Canbus0Send(uint32_t id, uint8_t *wdata, uint8_t len, uint8_t mode);
void Canbus0Recv(uint32_t *id, uint8_t *rdata, uint8_t *len);
int canbus0ReadIrq(int *id, uint8_t *rData, int32_t *queueLen);
int Canbus1Init(uint32_t baudrate);
void Canbus1IrqInit();
int Canbus1Send(uint32_t id, uint8_t *wdata, uint8_t len, uint8_t mode);
void Canbus1Recv(uint32_t *id, uint8_t *rdata, uint8_t *len);
int canbus1ReadIrq(int *id, uint8_t *rData, int32_t *queueLen);
void Canbus1_irq_Reset();
void Canbus0_irq_Reset();
void disable_can1();
void disable_can0();
void canbus1buffrest();
void canbus0buffrest();
#endif