#include "stm32f3xx.h"
/*
 * Note: This driver is specifically designed for the STM32F3
 *	discovery board. On the PCB, these functions will no
 *	longer work as intended.
 */
#ifndef SPARK_LED
#define SPARK_LED

void initLeds(void);
void ledOn(uint8_t led);
void ledOff(uint8_t led);
void ledToggle(uint8_t led);
void ledAllOn(void);
void ledAllOff(void);
void ledCircle(uint8_t numLeds);
void ledCircleInverted(uint8_t numLeds);

#endif
