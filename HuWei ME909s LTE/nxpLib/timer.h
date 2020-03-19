#ifndef NXPLIB_TIMER_H_
#define NXPLIB_TIMER_H_

void LPTMR_init(void);//2ms
int  TimerCreate(unsigned int time_2ms);
int  TimerDelete(unsigned int time_2ms);
int TimerOutGet(unsigned int time_2ms);

void LPIT0_init (void);//1ms
int  TimerLpitCreate(unsigned int time_ms);
int  TimerLpitDelete(unsigned int time_ms);
int TimerLpitOutGet(unsigned int time_ms);
#endif
