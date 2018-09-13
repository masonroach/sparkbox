#include "main.h"

int main(void)
{
	volatile uint32_t i;
	uint8_t j = 0, error = 0;
/*
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;

	GPIOB->MODER |= (GPIO_MODER_MODE2_0);
	GPIOC->MODER |= (GPIO_MODER_MODE0_0);
*/
	initLeds();

	while(1){/*
		GPIOB->ODR ^= GPIO_ODR_OD2;
		GPIOC->ODR ^= GPIO_ODR_OD0;
*/
		if (++j > 8) {
			j = 0;
			error = ++error > 2 ? 0 : error;
			ledError(error);
		}
		ledMap(0xFF >> (8 - j));
		for(i=0; i < 100000; i++);
	}
}
