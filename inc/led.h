/*! 
 * @file led.h
 * @author Mason Roach
 * @author Patrick Roy
 * @date Dec 12 2018
 *
 * @brief Functions the control the LEDs on the Sparkbox device
 *
 * These functions should be used to cotrol the 8 general purpose LEDs on the
 * Sparkbox, as well as the two debugging LEDs.
 */

#include "stm32f4xx.h"

#ifndef SPARK_LED
#define SPARK_LED

/*!
 * @brief Enumerable used with ledError function
 */
typedef enum {
	LED_OFF		= 0,	/*!< Turns both LEDs off */
	LED_WARNING	= 1,	/*!< Controls the yellow LED */
	LED_ERROR	= 2		/*!< Controls the red LED */
} LED_DEBUG;

/*!
 * @brief Initializes LED hardware
 *
 * Initializes the proper registers on the STM32F4 to be outputs
 */
void initLeds(void);

/*!
 * @brief Turns on a desginated LED
 *
 * @param led Designates which LED to turn on
 */
void ledOn(uint8_t led);

/*!
 * @brief Turn off a designated LED
 *
 * @param led Designates which LED to turn off
 */
void ledOff(uint8_t led);

/*!
 * @brief Toggle a designated LED
 *
 * @param led Designates which LED to turn toggle
 */
void ledToggle(uint8_t led);

/*!
 * @brief Turn all general purpose LEDs on
 */
void ledAllOn(void);

/*!
 * @brief Turn all general purpose LEDs off
 */
void ledAllOff(void);

/*!
 * @brief Set the error LED set
 *
 * The error LED set has three possible states: both off, yellow on, or red on.
 * The function will set the state of these LEDs to the given input state.
 * @see LED_DEBUG
 *
 * @param status Designate which state the LED set should be in
 */
void ledError(LED_DEBUG status);

/*!
 * @brief Input a 8-bit map to turn the corresponding LED's on and off
 *
 * The input value will be used as a bitmap to enable or disable the general
 * purpose LEDs on the Sparkbox. Values of 1 will turn on the LED, values of 0
 * will turn off the LED.
 *
 * @param map The bitmap to push to the LEDs
 */
void ledMap(uint8_t map);
#endif
