#include "led.h"

// Initialize all 8 LEDs
void initLeds(void) {
	RCC->AHBENR    |=   RCC_AHBENR_GPIOEEN;	// Enable GPIOE Clock

	GPIOE->MODER   |=   0x5555 << (8 * 2);	// GPIO Modes -> output
	GPIOE->OTYPER  &= ~(0xFF   << (8 * 1));	// GPIO OType -> open drain
	GPIOE->PUPDR   |=   0xAAAA << (8 * 2);	// GPIO PU/PD -> No PU/PD
	GPIOE->OSPEEDR |=   0x5555 << (8 * 2);	// GPIO Speed -> Medium
}

// Turns on a desginated LED
void ledOn(uint8_t led) {
	GPIOE->BSRR |= 1 << (8 + led);		// Turn on led
}

// Turn off a designated LED
void ledOff(uint8_t led) {
	GPIOE->BSRR |= 1 << (8 + 16 + led);	// Turn off led
}

// Toggle a designated LED
void ledToggle(uint8_t led) {
	GPIOE->ODR ^= 1 << (8 + led);
}

// Turn all LEDs on
void ledAllOn(void) {
	GPIOE->BSRR |= 0xFF << 8;
}

// Turn all LEDs off
void ledAllOff(void) {
	GPIOE->BSRR |= 0xFF << (8 + 16);
}

// Turn on LEDs in a circle
void ledCircle(uint8_t numLeds) {
	ledAllOff();
	GPIOE->BSRR |= (0x00FF << numLeds+1) & 0xFF00;
	if (numLeds > 7) {
		ledOn(0);
	} else {
		ledOff(0);
	}
}
