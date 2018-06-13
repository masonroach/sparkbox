#include "led.h"

// Initialize all 8 LEDs
void initLeds(void) {
	RCC->AHB1ENR    |=   RCC_AHB1ENR_GPIODEN;	// Enable GPIOD Clock

	GPIOD->MODER   |=   0x55 << (12 * 2);	// GPIO Modes -> output
	GPIOD->OTYPER  &= ~(0xFF << (12 * 1));	// GPIO OType -> open drain
	GPIOD->PUPDR   |=   0xAA << (12 * 2);	// GPIO PU/PD -> No PU/PD
	GPIOD->OSPEEDR |=   0x55 << (12 * 2);	// GPIO Speed -> Medium
}

// Turns on a desginated LED
void ledOn(LEDCOLOR led) {
	GPIOD->BSRR |= 1 << (12 + led);		// Turn on led
}

// Turn off a designated LED
void ledOff(LEDCOLOR led) {
	GPIOD->BSRR |= 1 << (12 + 16 + led);	// Turn off led
}

// Toggle a designated LED
void ledToggle(LEDCOLOR led) {
	GPIOD->ODR ^= 1 << (12 + led);
}

// Turn all LEDs on
void ledAllOn(void) {
	GPIOD->BSRR |= 0xF << 12;
}

// Turn all LEDs off
void ledAllOff(void) {
	GPIOD->BSRR |= 0xF << (12 + 16);
}

// Turn on LEDs in a clockwise circle
void ledCircle(uint8_t numLeds) {
	ledAllOff();
	GPIOD->BSRR |= ((0x0F00 << (numLeds+1)) & 0xE000);
	if (numLeds > 3) {
		ledOn(GREEN);
	} else {
		ledOff(GREEN);
	}
}

// Turn on LEDs in an inverted clockwise cirlce
void ledCircleInverted(uint8_t numLeds) {
	ledAllOff();
	GPIOD->BSRR |= (~(0x0F00 << (numLeds+1)) & 0xE000);
	if (numLeds > 3) {
		ledOff(GREEN);
	} else {
		ledOn(GREEN);
	}
}
