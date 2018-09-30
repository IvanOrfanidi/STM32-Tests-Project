

#ifndef __TIMER_H
#define __TIMER_H

#define TIMER2_PERIOD 1
#define TIMER3_PERIOD 1
#define TIMER4_PERIOD 1000
#define TIMER7_PERIOD 10000
#define TIMER6_PERIOD 1000
#define TIMER9_PERIOD 2000

void TM3DeInit(void);

void TM3Init(void);   // Timer delay(no use RTOS).
void TM4Init(void);   // Timer Led.
void TM6Init(void);   // Timer Buzzer.
void TM9Init(void);   // Timer PWR Buzzer.
void TM7Init(void);   // Timer One Sec.
void TIM3_IRQHandler(void);
void TIM5_IRQHandler(void);
int isEndWaitPeriod(const uint32_t wait);
int isEndWaitPeriodStopRTOS(uint32_t wait);
uint32_t get_cur_sec(void);

#endif