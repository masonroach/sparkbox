#include "stm32f4xx.h"

#ifndef SPARK_RNG
#define SPARK_RNG

void initRng(void);
uint32_t rand32(void);

#endif
