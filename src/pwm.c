#include "pwm.h"

AUDIO_T *audio;
// Initialize PWM output
int initAudio(void){
	int i;

	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;	// Enable GPOIB Clock

	// Allocate memory for audio struct
	audio->cosTable = initMyCos();
	audio->frequency = (int *)malloc(sizeof(int)*4);
	audio->volume = (unsigned char *)malloc(sizeof(unsigned char)*4);
	if (audio->cosTable == NULL || audio->frequency == NULL || audio->volume == NULL) return -1;

	for(i=0; i<4; i++){
		(audio->frequency)[i] = 0;
		(audio->volume)[i] = 0;
	}

	// Set PB6-9 alternate function Mode
	GPIOB->MODER |= GPIO_MODER_MODER6_1 | GPIO_MODER_MODER7_1 | GPIO_MODER_MODER8_1 | GPIO_MODER_MODER9_1;

	// PB6 TIM4_CH1, PB7 TIM4_CH2, PB8 TIM4_CH3, PB9 TIM4_CH4
	GPIOB->AFR[0] |= (0x2 << GPIO_AFRL_AFRL6_Pos) | (0x2 << GPIO_AFRL_AFRL7_Pos);
	GPIOB->AFR[1] |= (0x2 << GPIO_AFRH_AFRH0_Pos) | (0x2 << GPIO_AFRH_AFRH1_Pos);

	// 100 Hz to 3 KHz range needed on sine wave
	// drive PWM at 50 kHz
	// lowpass filter with cutoff frequency at 5 kHz

	// Enable clock to timer 4
	RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;

	// Select upcounting
	TIM4->CR1 &= ~TIM_CR1_DIR;

	// For 8 MHz clock
	TIM4->PSC &= ~0xFFFF; // Clear prescaler register
	TIM4->ARR = 159; // 8 MHz / PSC / (ARR + 1) = 50e3

	// Initialize interrupts to occur at PWM frequency

	// Channels 1-4 PWM Mode 2
	TIM4->CCMR1 &= ~TIM_CCMR1_OC1M | TIM_CCMR1_OC2M;
	TIM4->CCMR2 &= ~TIM_CCMR2_OC3M | TIM_CCMR2_OC4M;
	TIM4->CCMR1 |= TIM_CCMR1_OC1M_0 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2;
	TIM4->CCMR1 |= TIM_CCMR1_OC2M_0 | TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2;
	TIM4->CCMR2 |= TIM_CCMR2_OC3M_0 | TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2;
	TIM4->CCMR2 |= TIM_CCMR2_OC4M_0 | TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_2;

	// Output compare preload enabled
	TIM4->CCMR1 |= TIM_CCMR1_OC1PE | TIM_CCMR1_OC2PE;
	TIM4->CCMR2 |= TIM_CCMR2_OC3PE | TIM_CCMR2_OC4PE;

	// Auto-reload preload enabled
	TIM4->CR1 |= TIM_CR1_ARPE;

	// Enable update interrupt for timer 4
	TIM4->DIER |= TIM_DIER_UIE;

	// Counter enabled
	TIM4->CR1 |= TIM_CR1_CEN;

	// Enable interrupts for timer 4
	NVIC_EnableIRQ(TIM4_IRQn);

	return 0;
}

// Initialize fast cosine function, create lookup table
float* initMyCos(void){
	float *cosTable;
	int i;

	// Allocate 50 float array for cosine table
	cosTable = (float *)malloc(sizeof(float) * 50);

	// Fill table if not NULL
	if (cosTable != NULL){
		for(i=0; i<50; i++){
			cosTable[i] = 0.5 * cos(PIFIFTY * (float)i) + 0.5;
		}
	}

	// returns pointer to cosTable array on success, NULL on failure
	return cosTable;
}

// Valid frequencies range from 100-3000
void setFrequency(int *freq){
	audio->frequency[0] = freq[0];
	audio->frequency[1] = freq[1];
	audio->frequency[2] = freq[2];
	audio->frequency[3] = freq[3];
}

// Volume represented as a percent (0-100)
void setVolume(unsigned char *vol){
	audio->volume[0] = vol[0];
	audio->volume[1] = vol[1];
	audio->volume[2] = vol[2];
	audio->volume[3] = vol[3];
}

void destroyAudio(void){
	free(audio->cosTable);
	free(audio->frequency);
	free(audio->volume);
	return;
}

void TIM4_IRQHandler(void){
	static int n = 0;
	TIM4->CCR1 = (int)round(TIM4->ARR * audio->volume[0] * 0.01 * myCos(TWOPI * audio->frequency[0] / 50000.0 * n));
	TIM4->CCR2 = (int)round(TIM4->ARR * audio->volume[1] * 0.01 * myCos(TWOPI * audio->frequency[1] / 50000.0 * n));
	TIM4->CCR3 = (int)round(TIM4->ARR * audio->volume[2] * 0.01 * myCos(TWOPI * audio->frequency[2] / 50000.0 * n));
	TIM4->CCR4 = (int)round(TIM4->ARR * audio->volume[3] * 0.01 * myCos(TWOPI * audio->frequency[3] / 50000.0 * n));
	if (++n > 49999) n = 0;
}

// Fast verison of cosine using lookup table
// Returns value between 0 and 1
float myCos(float x){
	float testVal;
	int i;

	// Absolute value because cos(x) = cox(-x)
	if (x < 0.0) x *= -1.0;
	// Adjust x to beneath 2pi
	if (x > TWOPI){
		x = x - (TWOPI * (int)(x / TWOPI));
	}
	// Map x to value between 0 and 2PI
	if (x > PI) x = TWOPI - x;

	// Round x to nearest multiple of PI/50
	testVal = PIFIFTY;
	for (i=0; i<50; i++, testVal += PIFIFTY){
		if (x < testVal) return audio->cosTable[i];
	}
	// In the unlikely case
	return audio->cosTable[49];
}
