#ifndef SPARK_CLK
#define SPARK_CLK

#define CLOCK_FREQ 168000000UL

#include "stm32f4xx.h"
#include "core_cm4.h"

uint8_t initSystemClock(void);
void delayms(uint16_t ms);
uint8_t SystemClock_Config(void);
void SysTick_Handler(void);

#endif
