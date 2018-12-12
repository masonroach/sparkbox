/*!
 * @file led.c
 * @author Mason Roach
 * @author Patrick Roy
 * @date Dec 12 2018
 *
 * @brief Functions the control the LEDs on the Sparkbox device
 *
 * These functions should be used to cotrol the 8 general purpose LEDs on the
 * Sparkbox, as well as the two debugging LEDs.
 */

#include "led.h"

/*!
 * @brief Initializes LED hardware
 *
 * Initializes the proper registers on the STM32F4 to be outputs
 */
void initLeds(void) {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN |	// Enable GPIOC Clock
					RCC_AHB1ENR_GPIOBEN;	// Enable GPIOB Clock

	GPIOC->MODER  |=  0x5555;	// GPIO Modes -> output
	GPIOC->OTYPER &= ~0xFF;		// GPIO OType -> open drain

	GPIOB->OTYPER &= ~GPIO_OTYPER_OT_0;		// Open drain
}

/*!
 * @brief Turns on a desginated LED
 *
 * @param led Designates which LED to turn on
 */
void ledOn(uint8_t led) {
	GPIOC->ODR |= 1 << led;	// Turn on led
}

/*!
 * @brief Turn off a designated LED
 *
 * @param led Designates which LED to turn off
 */
void ledOff(uint8_t led) {
	GPIOC->ODR &= ~(1 << led);	// Turn off led
}

/*!
 * @brief Toggle a designated LED
 *
 * @param led Designates which LED to turn toggle
 */
void ledToggle(uint8_t led) {
	GPIOC->ODR ^= 1 << led;
}

/*!
 * @brief Turn all general purpose LEDs on
 */
void ledAllOn(void) {
	GPIOC->ODR |= 0x00FF;
}

/*!
 * @brief Turn all general purpose LEDs off
 */
void ledAllOff(void) {
	GPIOC->ODR &= 0xFF00;
}

/*!
 * @brief Set the error LED set
 *
 * The error LED set has three possible states: both off, yellow on, or red on.
 * The function will set the state of these LEDs to the given input state.
 * @see LED_DEBUG
 *
 * @param status Designate which state the LED set should be in
 */
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

/*!
 * @brief Input a 8-bit map to turn the corresponding LED's on and off
 *
 * The input value will be used as a bitmap to enable or disable the general
 * purpose LEDs on the Sparkbox. Values of 1 will turn on the LED, values of 0
 * will turn off the LED.
 *
 * @param map The bitmap to push to the LEDs
 */
void ledMap(uint8_t map) {
	GPIOC->BSRR |= (map & 0xFF) + ((~map & 0xFF) << 16);
}
