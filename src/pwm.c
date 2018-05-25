/*#include "pwm.h"

// Initialize PWM output
void initPWM(void){
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;	// Enable GPOIB Clock

	pwmPinInit();

	tim4init();
}

// Initialize pins used by audio driver as channels of general purpose timer 44
void pwmPinInit(void) {
	// Set PB6-9 alternate function Mode
	GPIOB->MODER |= GPIO_MODER_MODER6_1 | GPIO_MODER_MODER7_1 | GPIO_MODER_MODER8_1 | GPIO_MODER_MODER9_1;

	// PB6 TIM4_CH1, PB7 TIM4_CH2, PB8 TIM4_CH3, PB9 TIM4_CH4
	GPIOB->AFRL |= (0x2 << GPIO_AFRL_AFRL6_Pos) | (0x2 << GPIO_AFRL_AFRL7_Pos);
	GPIOB->AFRH |= (0x2 << GPIO_AFRH_AFRH8_Pos) | (0x2 << GPIO_AFRH_AFRH9_Pos);
}


// Initialize Timer 4 as
void tim4init(void){
	// 100 Hz to 3 KHz range needed on sine wave
	// drive PWM at 300 kHz
	// lowpass filter with cutoff frequency at 10 kHz to attenuate

}
*/
