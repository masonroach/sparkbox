#include "stm32f4xx.h"
/*
 * Note: This driver is specifically designed for the STM32F3
 *	discovery board. On the PCB, these functions will no
 *	longer work as intended.
 */
#ifndef SPARK_LED_H
#define SPARK_LED_H

// LED functions for the discovery, not the sparbox
/*
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
*/

// LED functions for the sparkbox, not the discovery
typedef enum {
	LED_OFF		= 0,
	LED_WARNING	= 1,
	LED_ERROR	= 2
} LED_DEBUG;

void initLeds(void);
void ledOn(uint8_t led);
void ledOff(uint8_t led);
void ledToggle(uint8_t led);
void ledAllOn(void);
void ledAllOff(void);
void ledError(LED_DEBUG status);
void ledMap(uint8_t map);
#endif
