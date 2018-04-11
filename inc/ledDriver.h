#include "stm32f3xx.h"
/*
 * Note: This driver is specifically designed for the STM32F3
 *	discovery board. On the PCB, these functions will no
 *	longer work as intended.
 */
void initLeds(void);
void ledOn(uint8_t led);
void ledOff(uint8_t led);
void ledToggle(uint8_t led);
void ledAllOn(void);
void ledAllOff(void);
void ledCircle(uint8_t numLeds);
