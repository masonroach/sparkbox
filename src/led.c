#include "led.h"

void initLeds(void) {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN |	// Enable GPIOC Clock
					RCC_AHB1ENR_GPIOBEN;	// Enable GPIOB Clock

	GPIOC->MODER  |=  0x5555;	// GPIO Modes -> output
	GPIOC->OTYPER &= ~0xFF;		// GPIO OType -> open drain

	GPIOB->OTYPER &= ~GPIO_OTYPER_OT_0;		// Open drain
}

// Turns on a desginated LED
void ledOn(uint8_t led) {
	GPIOC->ODR |= 1 << led;	// Turn on led
}

// Turn off a designated LED
void ledOff(uint8_t led) {
	GPIOC->ODR &= ~(1 << led);	// Turn off led
}

// Toggle a designated LED
void ledToggle(uint8_t led) {
	GPIOC->ODR ^= 1 << led;
}

// Turn all LEDs on
void ledAllOn(void) {
	GPIOC->ODR |= 0x00FF;
}

// Turn all LEDs off
void ledAllOff(void) {
	GPIOC->ODR &= 0xFF00;
}

// Sets the error led off (0), yellow (1), or red (2)
void ledError(LED_DEBUG status) {
	if (status) {	// Enabled, either yellow or red
		GPIOB->MODER |= GPIO_MODER_MODE0_0;	// Set pin as output
		if (status == LED_WARNING) {	// Warning, yellow
			GPIOB->ODR &= ~1;
		} else {	// Error, red
			GPIOB->ODR |= 1;
		}
	} else {		// Disabled
		GPIOB->MODER &= ~GPIO_MODER_MODE0;	// Set pin as input
	}
}

// Input a 8-bit map to turn the corresponding LED's on and off
uint8_t ledMap(uint8_t map) {
	GPIOC->BSRR |= (map & 0xFF) + ((~map & 0xFF) << 16);

	return map;
}
