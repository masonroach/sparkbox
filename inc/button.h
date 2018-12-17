/*!
 * @file button.h
 * @author Mason Roach
 * @author Patrick Roy
 * @date Dec 13 2018
 *
 * @brief Functions and define statements to interact with the buttons
 *
 * After initializing the buttons, the interrupt handlers will automatically
 * update the values of all buttons except the start button as they change
 * state. 
 *
 */

#ifndef SPARK_BUTTON
#define SPARK_BUTTON
#include "stm32f4xx.h"

/*!
 * @brief Give access to buttons variable to any file including this one
 */
extern volatile uint8_t buttons;

/*!
 * @brief Macros for waiting until a button is pushed or released
 */
#define WAIT_UNTIL_PUSH(__BUTTON_) 	while(__BUTTON__ == RELEASED)
#define WAIT_UNTIL_RELEASE(__BUTTON__) while(__BUTTON__ == PUSHED)

/*!
 * @brief define statements to access a button's bit for each button
 */
#define BUTTON_A        ((buttons >> 0) & 0x01)
#define BUTTON_B        ((buttons >> 1) & 0x01)
#define BUTTON_X        ((buttons >> 2) & 0x01)
#define BUTTON_Y        ((buttons >> 3) & 0x01)
#define BUTTON_DOWN     ((buttons >> 4) & 0x01)
#define BUTTON_UP       ((buttons >> 5) & 0x01)
#define BUTTON_RIGHT    ((buttons >> 6) & 0x01)
#define BUTTON_LEFT     ((buttons >> 7) & 0x01)

/*!
 * @brief Pushed and released defines for code readability
 */
#define PUSHED 0x01
#define RELEASED 0x00

/*!
 * @brief Initialize all buttons
 *
 * This function initializes the button GPIO pins to inputs and configures
 * external interrupts to trigger when the state of the left, right, up, down,
 * a, b, x, or y buttons change.
 *
 */
void initButtons(void);

/*!
 * @brief Read the status of the start button
 *
 * @return 0 for not pushed, 1 for pushed
 */
uint8_t readButton(void);

#endif
