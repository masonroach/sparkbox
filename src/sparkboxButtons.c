#include "sparkboxButtons.h"


volatile bool BUTTON_LEFT = 0; 	// PA1
volatile bool BUTTON_RIGHT = 0; // PA2
volatile bool BUTTON_UP = 0;		// PA3
volatile bool BUTTON_DOWN = 0;	// PA4
volatile bool BUTTON_A = 0;			// PA5
volatile bool BUTTON_B = 0;			// PA6
volatile bool BUTTON_X = 0;			// PA7
volatile bool BUTTON_Y = 0;			// PA8

void initButtons(void){
	// Sparkbox buttons are PA1-8
	uint32_t priorityGroup, priority;

	// Enable clock of PORTA and SYSCFG
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

	// Select PORTA as EXTICR1-8
	SYSCFG->EXTICR[0] &= ~(SYSCFG_EXTICR1_EXTI0 | SYSCFG_EXTICR1_EXTI1 | 
		SYSCFG_EXTICR1_EXTI2); // PA1-3
	SYSCFG->EXTICR[1] &= ~(SYSCFG_EXTICR2_EXTI4 | SYSCFG_EXTICR2_EXTI5 | 
		SYSCFG_EXTICR2_EXTI6 | SYSCFG_EXTICR2_EXTI7); // PA4-7
	SYSCFG->EXTICR[2] &= ~(SYSCFG_EXTICR3_EXTI8); // PA8

	// Set all pins to inputs
	GPIOA->MODER &= ~(GPIO_MODER_MODER1 | GPIO_MODER_MODER2 | GPIO_MODER_MODER3);
	GPIOA->MODER &= ~(GPIO_MODER_MODER4 | GPIO_MODER_MODER5 | GPIO_MODER_MODER6);
	GPIOA->MODER &= ~(GPIO_MODER_MODER7 | GPIO_MODER_MODER8);


	// Unmask EXTI interrupts
	EXTI->IMR |= EXTI_IMR_MR1 | EXTI_IMR_MR2 | EXTI_IMR_MR3 | EXTI_IMR_MR4
			| EXTI_IMR_MR5 | EXTI_IMR_MR6 | EXTI_IMR_MR7 | EXTI_IMR_MR8;


	// Configure rising or falling edge to trigger interrupts
	// Rising edges
	EXTI->RTSR |= EXTI_RTSR_TR1 | EXTI_RTSR_TR2 | EXTI_RTSR_TR3 |
				EXTI_RTSR_TR4 | EXTI_RTSR_TR5 | EXTI_RTSR_TR6 |
				EXTI_RTSR_TR7 | EXTI_RTSR_TR8;
	// Falling edges
	EXTI->FTSR |= EXTI_FTSR_TR1 | EXTI_FTSR_TR2 | EXTI_FTSR_TR3 |
				EXTI_FTSR_TR4 | EXTI_FTSR_TR5 | EXTI_FTSR_TR6 |
				EXTI_FTSR_TR7 | EXTI_FTSR_TR8;

	// Get initial values
	BUTTON_LEFT = ((GPIOA->IDR & GPIO_IDR_IDR_1) != 0); 	// PA1
	BUTTON_RIGHT = ((GPIOA->IDR & GPIO_IDR_IDR_2) != 0);  // PA2
	BUTTON_UP = ((GPIOA->IDR & GPIO_IDR_IDR_3) != 0);		// PA3
	BUTTON_DOWN = ((GPIOA->IDR & GPIO_IDR_IDR_4) != 0);	// PA4
	BUTTON_A = ((GPIOA->IDR & GPIO_IDR_IDR_5) != 0);			// PA5
	BUTTON_B = ((GPIOA->IDR & GPIO_IDR_IDR_6) != 0);			// PA6
	BUTTON_X = ((GPIOA->IDR & GPIO_IDR_IDR_7) != 0);			// PA7
	BUTTON_Y = ((GPIOA->IDR & GPIO_IDR_IDR_8) != 0);			// PA8

	// Set priority Number
	priorityGroup = NVIC_GetPriorityGrouping();
	priority = NVIC_EncodePriority(priorityGroup, 2, 1 );
	NVIC_SetPriority(EXTI1_IRQn, priority);

	// Set priority Number
	priorityGroup = NVIC_GetPriorityGrouping();
	priority = NVIC_EncodePriority(priorityGroup, 2, 1 );
	NVIC_SetPriority(EXTI2_IRQn, priority);
	
	// Set priority Number
	priorityGroup = NVIC_GetPriorityGrouping();
	priority = NVIC_EncodePriority(priorityGroup, 2, 1 );
	NVIC_SetPriority(EXTI3_IRQn, priority);
	
	// Set priority Number
	priorityGroup = NVIC_GetPriorityGrouping();
	priority = NVIC_EncodePriority(priorityGroup, 2, 1 );
	NVIC_SetPriority(EXTI4_IRQn, priority);
	
	// Set priority Number
	priorityGroup = NVIC_GetPriorityGrouping();
	priority = NVIC_EncodePriority(priorityGroup, 2, 1 );
	NVIC_SetPriority(EXTI9_5_IRQn, priority);
  
	// Enable relevant EXTI interrupts
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
	BUTTON_DOWN = ((GPIOA->IDR & GPIO_IDR_IDR_4) != 0);	// PA4
}

void EXTI9_5_IRQHandler(void){
	// ack correct interrupts, update button values
	if (EXTI->PR & EXTI_PR_PR5) {
		EXTI->PR |= EXTI_PR_PR5;
		BUTTON_LEFT = ((GPIOA->IDR & GPIO_IDR_IDR_5) != 0); 	// PA5
	}
	if (EXTI->PR & EXTI_PR_PR6) {
		EXTI->PR |= EXTI_PR_PR6;
		BUTTON_LEFT = ((GPIOA->IDR & GPIO_IDR_IDR_6) != 0); 	// PA6
	}
	if (EXTI->PR & EXTI_PR_PR7) {
		EXTI->PR |= EXTI_PR_PR7;
		BUTTON_LEFT = ((GPIOA->IDR & GPIO_IDR_IDR_7) != 0); 	// PA7
	}
	if (EXTI->PR & EXTI_PR_PR8) {
		EXTI->PR |= EXTI_PR_PR8;
		BUTTON_LEFT = ((GPIOA->IDR & GPIO_IDR_IDR_8) != 0); 	// PA8
	}
}
