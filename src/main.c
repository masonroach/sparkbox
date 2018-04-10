#include "stm32f3xx.h"
//#include "usartDriver.h"

int main(void) {
/*	int8_t i = 0;
	int8_t *s = (int8_t *)" Testing . . .\n";

	RCC->CR |= (RCC_CR_HSION);
	
	while ((RCC->CR & RCC_CR_HSIRDY) == 0);	// Wait for HSI to be ready
	RCC->CFGR &= ~RCC_CFGR_SW;		// Select HSI as clock
	RCC->CFGR |=  RCC_CFGR_SW_HSI;
	while ((RCC->CFGR & RCC_CFGR_SWS) == 0);
*/



	RCC->AHBENR    |=   RCC_AHBENR_GPIOEEN;		// Enable GPIOE Clock
	GPIOE->MODER   |=   GPIO_MODER_MODER8_0;	// GPIO Mode output
	GPIOE->OTYPER  &=  ~GPIO_OTYPER_OT_8;		// GPIO OType OD
	GPIOE->PUPDR   |=   GPIO_PUPDR_PUPDR8_1;	
	GPIOE->OSPEEDR |=   GPIO_OSPEEDER_OSPEEDR8_0;	// GPIO Speed

	GPIOE->BSRR    |=   GPIO_BSRR_BS_8;		// Turn on led




/*
	gpioConfig();
	usartConfig();
	usartSendChar('T');
	while (1) {
		usartSendString(s);
		usartSendChar(i);
	}
*/
	return 1;
}
