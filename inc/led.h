#include "stm32f4xx.h"
/*
 * Note: This driver is specifically designed for the STM32F3
 *	discovery board. On the PCB, these functions will no
 *	longer work as intended.
 */
#ifndef SPARK_LED
#define SPARK_LED

typedef enum {
	GREEN	= 0,	// PD12
	ORANGE	= 1,	// PD13
	RED		= 2,	// PD14
	BLUE	= 3		// PD15
} LEDCOLOR;

void initLeds(void);
void ledOn(LEDCOLOR led);
void ledOff(LEDCOLOR led);
void ledToggle(LEDCOLOR led);
void ledAllOn(void);
void ledAllOff(void);
void ledCircle(uint8_t numLeds);
void ledCircleInverted(uint8_t numLeds);

#endif
