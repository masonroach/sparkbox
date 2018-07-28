#include "sparkboxButtons.h"


volatile bool BUTTON_LEFT = 0; 	// PA1
volatile bool BUTTON_RIGHT = 0; // PA2
volatile bool BUTTON_UP = 0;		// PA3
volatile bool BUTTON_DOWN = 0;	// PB4
volatile bool BUTTON_A = 0;			// PA5
volatile bool BUTTON_B = 0;			// PA6
volatile bool BUTTON_X = 0;			// PA7
volatile bool BUTTON_Y = 0;			// PA8

void initButtons(void){

	// Enable clock of PORTB and SYSCFG
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

	NVIC_DisableIRQ(EXTI1_IRQn);
	NVIC_DisableIRQ(EXTI2_IRQn);
	NVIC_DisableIRQ(EXTI3_IRQn);
	NVIC_DisableIRQ(EXTI4_IRQn);
	NVIC_DisableIRQ(EXTI9_5_IRQn); // EXTI lines 1-9
	
	// Select PORTB as EXTICR1-8
	SYSCFG->EXTICR[0] &= ~(0xFFF0); //PA1-3
	
	SYSCFG->EXTICR[1] &= ~(0x000F);
	SYSCFG->EXTICR[1] |= 0x0001; // PB4, PA5-8

	// Set all button pins to inputs
	GPIOA->MODER &= ~(GPIO_MODER_MODER0 | GPIO_MODER_MODER1 | GPIO_MODER_MODER2);
	GPIOA->MODER &= ~(GPIO_MODER_MODER3 | GPIO_MODER_MODER5);
	GPIOA->MODER &= ~(GPIO_MODER_MODER6 | GPIO_MODER_MODER7);
	GPIOB->MODER &= ~(GPIO_MODER_MODER4);
	
	// Unmask EXTI interrupts
	EXTI->IMR |= EXTI_IMR_MR8 | EXTI_IMR_MR1 | EXTI_IMR_MR2 | EXTI_IMR_MR3
			| EXTI_IMR_MR4 | EXTI_IMR_MR5 | EXTI_IMR_MR6 | EXTI_IMR_MR7;


	// Configure rising or falling edge to trigger interrupts
	// Rising edges
	EXTI->RTSR |= EXTI_RTSR_TR8 | EXTI_RTSR_TR1 | EXTI_RTSR_TR2 |
				EXTI_RTSR_TR3 | EXTI_RTSR_TR4 | EXTI_RTSR_TR5 |
				EXTI_RTSR_TR6 | EXTI_RTSR_TR7;
	// Falling edges
	EXTI->FTSR |= EXTI_FTSR_TR0 | EXTI_FTSR_TR1 | EXTI_FTSR_TR2 |
				EXTI_FTSR_TR3 | EXTI_FTSR_TR4 | EXTI_FTSR_TR5 |
				EXTI_FTSR_TR6 | EXTI_FTSR_TR7;

	// Get initial values
	BUTTON_LEFT = ((GPIOA->IDR & GPIO_IDR_IDR_1) != 0); 	// PA1
	BUTTON_RIGHT = ((GPIOA->IDR & GPIO_IDR_IDR_2) != 0);  // PA2
	BUTTON_UP = ((GPIOA->IDR & GPIO_IDR_IDR_3) != 0);		// PA3
	BUTTON_DOWN = ((GPIOB->IDR & GPIO_IDR_IDR_4) != 0);	// PB4
	BUTTON_A = ((GPIOA->IDR & GPIO_IDR_IDR_5) != 0);			// PA5
	BUTTON_B = ((GPIOA->IDR & GPIO_IDR_IDR_6) != 0);			// PA6
	BUTTON_X = ((GPIOA->IDR & GPIO_IDR_IDR_7) != 0);			// PA7
	BUTTON_Y = ((GPIOA->IDR & GPIO_IDR_IDR_8) != 0);			// PA8

	NVIC_SetPriority(EXTI1_IRQn, 0x21);
	NVIC_SetPriority(EXTI2_IRQn, 0x22);
	NVIC_SetPriority(EXTI3_IRQn, 0x23);
	NVIC_SetPriority(EXTI4_IRQn, 0x24);
	NVIC_SetPriority(EXTI9_5_IRQn, 0x25);

	NVIC_EnableIRQ(EXTI1_IRQn);
	NVIC_EnableIRQ(EXTI2_IRQn);
	NVIC_EnableIRQ(EXTI3_IRQn);
	NVIC_EnableIRQ(EXTI4_IRQn);
	NVIC_EnableIRQ(EXTI9_5_IRQn); // EXTI lines 1-9
}



  
void EXTI1_IRQHandler(void) {
	// ack interrupt, update button values
	EXTI->PR |= EXTI_PR_PR1;
	BUTTON_LEFT = ((GPIOA->IDR & GPIO_IDR_IDR_1) != 0); 	// PA1
}

void EXTI2_IRQHandler(void) {
	// ack interrupt, update button values
	EXTI->PR |= EXTI_PR_PR2;
	BUTTON_RIGHT = ((GPIOA->IDR & GPIO_IDR_IDR_2) != 0);  // PA2
}

void EXTI3_IRQHandler(void) {
	// ack interrupt, update button values
	EXTI->PR |= EXTI_PR_PR3;
	BUTTON_UP = ((GPIOA->IDR & GPIO_IDR_IDR_3) != 0);		// PA3
}

void EXTI4_IRQHandler(void) {
	// ack interrupt, update button values
	EXTI->PR |= EXTI_PR_PR4;
	BUTTON_DOWN = ((GPIOB->IDR & GPIO_IDR_IDR_4) != 0);	// PB4
}

void EXTI9_5_IRQHandler(void){
	// ack correct interrupts, update button values
	if (EXTI->PR & EXTI_PR_PR5) {
		EXTI->PR |= EXTI_PR_PR5;
		BUTTON_A = ((GPIOA->IDR & GPIO_IDR_IDR_5) != 0); 	// PA5
	}
	if (EXTI->PR & EXTI_PR_PR6) {
		EXTI->PR |= EXTI_PR_PR6;
		BUTTON_B = ((GPIOA->IDR & GPIO_IDR_IDR_6) != 0); 	// PA6
	}
	if (EXTI->PR & EXTI_PR_PR7) {
		EXTI->PR |= EXTI_PR_PR7;
		BUTTON_X = ((GPIOA->IDR & GPIO_IDR_IDR_7) != 0); 	// PA7
	}
	if (EXTI->PR & EXTI_PR_PR8) {
		EXTI->PR |= EXTI_PR_PR8;
		BUTTON_Y = ((GPIOA->IDR & GPIO_IDR_IDR_8) != 0); 	// PA7
	}
}
