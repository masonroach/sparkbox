#include "stm32f4xx.h"
#include "led.h"

#ifndef SPARK_RAND
#define SPARK_RAND

void initRng(void);
uint32_t rand32(void);

#endif
