#include "button.h"

// Initialize the button (PA0) as an input
void initButton(void) {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;	// Enable GPIOA Clock
	
	GPIOA->MODER &= ~GPIO_MODER_MODER0;	// GPIO Mode Input
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT_0;	// GPIO OType -> Push-Pull
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR0_1;	// GPIO PU/PD -> Pull-down
}

// Read the value of the button
uint8_t readButton(void) {
	if (GPIOA->IDR & GPIO_IDR_IDR_0) {
		return 1;
	} else {
		return 0;
	}
}
