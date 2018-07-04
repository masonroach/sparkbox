#include "pushButton.h"

// PD 0bx011 for EXTICRx registers
#define PORTD_SYSCFG_MASK ((0b011 << 0) | (0b011 << 4) | (0b011 << 8) | (0b011 << 12))

volatile bool BUTTON_LEFT = 0; 	// PD8
volatile bool BUTTON_RIGHT = 0; // PD9
volatile bool BUTTON_UP = 0;		// PD10
volatile bool BUTTON_DOWN = 0;	// PD11
volatile bool BUTTON_A = 0;			// PD12
volatile bool BUTTON_B = 0;			// PD13
volatile bool BUTTON_X = 0;			// PD14
volatile bool BUTTON_Y = 0;			// PD15

void initButtons(void){
	// Sparkbox buttons are PD8-15
	uint32_t priorityGroup, priority;

	// Enable clock of PORTD and SYSCFG
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

	// Select PORTD as EXTICR8-15
	SYSCFG->EXTICR[2] |= PORTD_SYSCFG_MASK; // PD8-11
	SYSCFG->EXTICR[3] |= PORTD_SYSCFG_MASK; // PD12-15

	// Set all pins to inputs
	GPIOB->MODER &= ~(GPIO_MODER_MODER8 | GPIO_MODER_MODER9 | GPIO_MODER_MODER10);
	GPIOB->MODER &= ~(GPIO_MODER_MODER11 | GPIO_MODER_MODER12 | GPIO_MODER_MODER13);
	GPIOB->MODER &= ~(GPIO_MODER_MODER14 | GPIO_MODER_MODER15);


	// Unmask EXTI interrupts
	EXTI->IMR |= EXTI_IMR_MR8 | EXTI_IMR_MR9 | EXTI_IMR_MR10 | EXTI_IMR_MR11
			| EXTI_IMR_MR12 | EXTI_IMR_MR13 | EXTI_IMR_MR14 | EXTI_IMR_MR15;


	// Configure rising or falling edge to trigger interrupts
	// Rising edges
	EXTI->RTSR |= EXTI_RTSR_TR8 | EXTI_RTSR_TR9 | EXTI_RTSR_TR10 |
				EXTI_RTSR_TR11 | EXTI_RTSR_TR12 | EXTI_RTSR_TR13 |
				EXTI_RTSR_TR14 | EXTI_RTSR_TR15;
	// Falling edges
	EXTI->FTSR |= EXTI_FTSR_TR8 | EXTI_FTSR_TR9 | EXTI_FTSR_TR10 |
				EXTI_FTSR_TR11 | EXTI_FTSR_TR12 | EXTI_FTSR_TR13 |
				EXTI_FTSR_TR14 | EXTI_FTSR_TR15;

	// Get initial values
	BUTTON_LEFT = ((GPIOB->IDR & GPIO_IDR_IDR_8) != 0); 	// PD8
	BUTTON_RIGHT = ((GPIOB->IDR & GPIO_IDR_IDR_9) != 0);  // PD9
	BUTTON_UP = ((GPIOB->IDR & GPIO_IDR_IDR_10) != 0);		// PD10
	BUTTON_DOWN = ((GPIOB->IDR & GPIO_IDR_IDR_11) != 0);	// PD11
	BUTTON_A = ((GPIOB->IDR & GPIO_IDR_IDR_12) != 0);			// PD12
	BUTTON_B = ((GPIOB->IDR & GPIO_IDR_IDR_13) != 0);			// PD13
	BUTTON_X = ((GPIOB->IDR & GPIO_IDR_IDR_14) != 0);			// PD14
	BUTTON_Y = ((GPIOB->IDR & GPIO_IDR_IDR_15) != 0);			// PD15

	// Set priority Number
	priorityGroup = NVIC_GetPriorityGrouping();
  priority = NVIC_EncodePriority(priorityGroup, 1, 0 );
  NVIC_SetPriority(EXTI9_5_IRQn, priority);

	priorityGroup = NVIC_GetPriorityGrouping();
  priority = NVIC_EncodePriority(priorityGroup, 2, 1 );
	NVIC_SetPriority(EXTI15_10_IRQn, priority);

	// Enable relevant EXTI interrupts
	NVIC_EnableIRQ(EXTI9_5_IRQn); // EXTI lines 9-5
	NVIC_EnableIRQ(EXTI15_10_IRQn); // EXTI lines 10-15
}

void EXTI9_5_IRQHandler(void){
	// ack correct interrupts, update button values
	if (EXTI->PR & EXTI_PR_PR8){
		EXTI->PR |= EXTI_PR_PR8;
		BUTTON_LEFT = ((GPIOB->IDR & GPIO_IDR_IDR_8) != 0); 	// PD8
	}

	if (EXTI->PR & EXTI_PR_PR9){
		EXTI->PR |= EXTI_PR_PR9;
		BUTTON_RIGHT = ((GPIOB->IDR & GPIO_IDR_IDR_9) != 0);  // PD9
	}

}

void EXTI15_10_IRQHandler(void){
	// ack correct interrupts, update button values
	if (EXTI->PR & EXTI_PR_PR10){
		EXTI->PR |= EXTI_PR_PR10;
		BUTTON_UP = ((GPIOB->IDR & GPIO_IDR_IDR_10) != 0);		// PD10
	}

	if (EXTI->PR & EXTI_PR_PR11){
		EXTI->PR |= EXTI_PR_PR11;
		BUTTON_DOWN = ((GPIOB->IDR & GPIO_IDR_IDR_11) != 0);	// PD11
	}

	if (EXTI->PR & EXTI_PR_PR12){
		EXTI->PR |= EXTI_PR_PR12;
		BUTTON_A = ((GPIOB->IDR & GPIO_IDR_IDR_12) != 0);			// PD12
	}

	if (EXTI->PR & EXTI_PR_PR13){
		EXTI->PR |= EXTI_PR_PR13;
		BUTTON_B = ((GPIOB->IDR & GPIO_IDR_IDR_13) != 0);			// PD13
	}

	if (EXTI->PR & EXTI_PR_PR14){
		EXTI->PR |= EXTI_PR_PR14;
		BUTTON_X = ((GPIOB->IDR & GPIO_IDR_IDR_14) != 0);			// PD14
	}

	if (EXTI->PR & EXTI_PR_PR15){
		EXTI->PR |= EXTI_PR_PR15;
		BUTTON_Y = ((GPIOB->IDR & GPIO_IDR_IDR_15) != 0);			// PD15
	}
}
