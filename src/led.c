#include "led.h"
/*
#ifdef DISCOVERY
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
	GPIOD->ODR ^= 0x01 << (12 + led);
}

// Turn all LEDs on
void ledAllOn(void) {
	GPIOD->BSRR |= 0xF0 << 8;
}

// Turn all LEDs off
void ledAllOff(void) {
	GPIOD->BSRR |= 0xF0 << (8 + 16);
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
*/

void initLeds(void) {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN |	// Enable GPIOC Clock
					RCC_AHB1ENR_GPIOBEN;	// Enable GPIOB Clock

	GPIOC->MODER  |=  0x5555;	// GPIO Modes -> output
	GPIOC->OTYPER &= ~0xFFFF;	// GPIO OType -> open drain

//	GPIOB->MODER  |=  GPIO_MODER_MODE2_0;	// Output
	GPIOB->OTYPER &= ~GPIO_OTYPER_OT_2;		// Open drain
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
		GPIOB->MODER |= GPIO_MODER_MODE2_0;	// Set pin as output
		status -= 1;
		GPIOB->BSRR |= (status << 2) + (~status << 18);	// Set the color
	} else {		// Disabled
		GPIOB->MODER &= ~GPIO_MODER_MODE2;	// Set pin as input
	}
}

// Input a 8-bit map to turn the corresponding LED's on and off
void ledMap(uint8_t map) {
	GPIOC->BSRR |= (map & 0xFF) + ((~map & 0xFF) << 16);
}
