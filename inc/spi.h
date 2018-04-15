#include "stm32f3xx.h"

#ifndef SPARK_SPI
#define SPARK_SPI

#ifndef CLOCK_F
#define CLOCK_F 8000000UL
#endif

void spiInit(void);

#endif
