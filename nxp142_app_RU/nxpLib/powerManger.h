#ifndef POWERMANGER_H_
#define POWERMANGER_H_

void clkout_bus(void);
void init_SIRC(void);
void init_VLPR(void);
void enter_VLPR(void);
void enter_VLPS(void);
void enter_STOP(int mode);//1--stop1  2--stop2
void SlowRUN_to_VLPS(void);
void switch_to_SIRC_in_RUN(void);
void disable_FIRC_in_RUN(void);

#endif
