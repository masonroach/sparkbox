#include "rand.h"

void initRng(void) {
	RCC->AHB2ENR |= RCC_AHB2ENR_RNGEN;	// Enable core clock for RNG

	RNG->CR |= RNG_CR_RNGEN;	// Enable RNG peripheral
}

uint32_t rand32(void) {
	while (!(RNG->SR & RNG_SR_DRDY)) {	// Wait for data ready flag to be set
		if (RNG->SR & RNG_SR_SEIS) {ledOn(0); ledError(2);}
		if (RNG->SR & RNG_SR_CEIS) {ledOn(1); ledError(2);}
		if (RNG->SR & RNG_SR_SECS) {ledOn(2); ledError(2);}
		if (RNG->SR & RNG_SR_CECS) {ledOn(3); ledError(2);}
	}	

	return RNG->DR;
}
