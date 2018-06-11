#include "stm32f4xx.h"
/*
 * Note: This driver is specifically designed for the STM32F3
 *	discovery board. On the PCB, these functions will no
 *	longer work as intended.
 */
#ifndef SPARK_BUTTON
#define SPARK_BUTTON

void initButton(void);
uint8_t readButton(void);

#endif
