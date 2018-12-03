#include "button.h"

volatile uint8_t buttons = 0x00;

void initButtons(void){

    // Enable clock of PORTF, PORTA, and SYSCFG
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOFEN;
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

	// Initialize "Start" button (PA0) as input
	GPIOA->MODER &= ~GPIO_MODER_MODER0; // GPIO Mode Input
    GPIOA->OTYPER &= ~GPIO_OTYPER_OT_0; // GPIO OType -> Push-Pull
    GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR0; // GPIO PU/PD -> No Pull-up/down

    NVIC_DisableIRQ(EXTI0_IRQn);
    NVIC_DisableIRQ(EXTI1_IRQn);
    NVIC_DisableIRQ(EXTI2_IRQn);
    NVIC_DisableIRQ(EXTI3_IRQn);
    NVIC_DisableIRQ(EXTI4_IRQn);
    NVIC_DisableIRQ(EXTI9_5_IRQn); // EXTI lines 0-9

    // Select PORTF as EXTICR0-7
    SYSCFG->EXTICR[0] &= ~(0xFFFF); // PF0-3
    SYSCFG->EXTICR[1] &= ~(0xFFFF); // PF4-7

    SYSCFG->EXTICR[0] |= 0x5555; // PF0-3
    SYSCFG->EXTICR[1] |= 0x5555; // PF4-7

    // Set all button pins to inputs
    GPIOF->MODER &= ~(GPIO_MODER_MODER0 | GPIO_MODER_MODER1 | GPIO_MODER_MODER2
                    | GPIO_MODER_MODER3 | GPIO_MODER_MODER4 | GPIO_MODER_MODER5
                    | GPIO_MODER_MODER6 | GPIO_MODER_MODER7);

    // Unmask EXTI interrupts
    EXTI->IMR |= EXTI_IMR_MR0 | EXTI_IMR_MR1 | EXTI_IMR_MR2 | EXTI_IMR_MR3
            | EXTI_IMR_MR4 | EXTI_IMR_MR5 | EXTI_IMR_MR6 | EXTI_IMR_MR7;


    // Configure rising or falling edge to trigger interrupts
    // Rising edges
    EXTI->RTSR |= EXTI_RTSR_TR0 | EXTI_RTSR_TR1 | EXTI_RTSR_TR2 |
                EXTI_RTSR_TR3 | EXTI_RTSR_TR4 | EXTI_RTSR_TR5 |
                EXTI_RTSR_TR6 | EXTI_RTSR_TR7;
    // Falling edges
    EXTI->FTSR |= EXTI_FTSR_TR0 | EXTI_FTSR_TR1 | EXTI_FTSR_TR2 |
                EXTI_FTSR_TR3 | EXTI_FTSR_TR4 | EXTI_FTSR_TR5 |
                EXTI_FTSR_TR6 | EXTI_FTSR_TR7;

    // Get initial values
    buttons = (uint8_t)(GPIOF->IDR);

    NVIC_SetPriority(EXTI0_IRQn, 0x20);
    NVIC_SetPriority(EXTI1_IRQn, 0x21);
	NVIC_SetPriority(EXTI2_IRQn, 0x22);
    NVIC_SetPriority(EXTI3_IRQn, 0x23);
    NVIC_SetPriority(EXTI4_IRQn, 0x24);
    NVIC_SetPriority(EXTI9_5_IRQn, 0x25);

    NVIC_EnableIRQ(EXTI0_IRQn);
    NVIC_EnableIRQ(EXTI1_IRQn);
    NVIC_EnableIRQ(EXTI2_IRQn);
    NVIC_EnableIRQ(EXTI3_IRQn);
    NVIC_EnableIRQ(EXTI4_IRQn);
    NVIC_EnableIRQ(EXTI9_5_IRQn); // EXTI lines 1-9
}

// Read the value of the button
uint8_t readButton(void) {
	if (GPIOA->IDR & GPIO_IDR_IDR_0) {
		return 1;
	} else {
		return 0;
	}
}


// Interrupt handler for all buttons except start button
void EXTI0_IRQHandler(void) {
    // ack interrupt, update button values
    EXTI->PR |= EXTI_PR_PR0;
    // PF0, A
    buttons = (uint8_t)(GPIOF->IDR);
}

void EXTI1_IRQHandler(void) {
    // ack interrupt, update button values
    EXTI->PR |= EXTI_PR_PR1;
    // PF1, B
    buttons = (uint8_t)(GPIOF->IDR);
}

void EXTI2_IRQHandler(void) {
    // ack interrupt, update button values
    EXTI->PR |= EXTI_PR_PR2;
    // PF2, X
    buttons = (uint8_t)(GPIOF->IDR);
}

void EXTI3_IRQHandler(void) {
    // ack interrupt, update button values
    EXTI->PR |= EXTI_PR_PR3;
    // PF3, Y
    buttons = (uint8_t)(GPIOF->IDR);
}

void EXTI4_IRQHandler(void) {
    // ack interrupt, update button values
    EXTI->PR |= EXTI_PR_PR4;
    // PF4, DOWN
    buttons = (uint8_t)(GPIOF->IDR);
}

void EXTI9_5_IRQHandler(void){
    // ack correct interrupts, update button values
    if (EXTI->PR & EXTI_PR_PR5) {
        EXTI->PR |= EXTI_PR_PR5;
        // PF5, UP
    }
    if (EXTI->PR & EXTI_PR_PR6) {
        EXTI->PR |= EXTI_PR_PR6;
        // PF6, RIGHT
    }
    if (EXTI->PR & EXTI_PR_PR7) {
        EXTI->PR |= EXTI_PR_PR7;
        // PF7, LEFT
    }

    buttons = (uint8_t)(GPIOF->IDR);
}

