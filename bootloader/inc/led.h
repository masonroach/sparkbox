#include "stm32f4xx.h"

#ifndef SPARK_LED
#define SPARK_LED

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
uint8_t ledMap(uint8_t map);
#endif
