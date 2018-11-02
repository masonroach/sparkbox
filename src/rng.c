#include "rng.h"

void initRng(void) {
	RCC->AHB2ENR |= RCC_AHB2ENR_RNGEN;	// Enable core clock for RNG

	RNG->CR |= RNG_CR_RNGEN;	// Enable RNG peripheral
}

uint32_t rand32(void) {
	while (!(RNG->SR & RNG_SR_DRDY));	// Wait for data ready flag to be set

	return RNG->DR;		// Return random number
}
