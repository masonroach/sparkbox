#include "stm32f4xx.h"

#ifndef SPARK_CLK
#define SPARK_CLK

#define CLOCK_F 168000000UL

void initSystemClock(void);
void delayms(uint16_t ms);
void SystemClock_Config(void);

#endif
