#include "main.h"

int main(void)
{
	volatile int i;

	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;

	GPIOB->MODER |= (GPIO_MODER_MODE2_0);
	GPIOC->MODER |= (GPIO_MODER_MODE0_0);


	while(1){
		GPIOB->ODR ^= GPIO_ODR_OD2;
		GPIOC->ODR ^= GPIO_ODR_OD0;
		for(i=0; i < 100000; i++);
	}
}
