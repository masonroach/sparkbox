#include "spi.h"

void spiInit(void) {
	/*
	 * Enabling clocks
	 */
	RCC->AHBENR  |= RCC_AHBENR_GPIOBEN;	// Enable GPIOB clock
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;	// Enable SPI1 clock

	/*
	 * Configuring GPIOs
	 */
	GPIOB->MODER   |=  GPIO_MODER_MODER3_1 |	// GPIOB[5:3] Mode -> AF
					   GPIO_MODER_MODER4_1 |
					   GPIO_MODER_MODER5_1;
	GPIOA->AFR[0]  |= (5 << GPIO_AFRL_AFRL3_Pos) |	// Alternate function -> 5
					  (5 << GPIO_AFRL_AFRL4_Pos) |
					  (5 << GPIO_AFRL_AFRL5_Pos);
	GPIOA->OSPEEDR |=  GPIO_OSPEEDER_OSPEEDR3_0 |	// GPIO speed -> Medium
					   GPIO_OSPEEDER_OSPEEDR4_0 |
					   GPIO_OSPEEDER_OSPEEDR5_0;
	
	/*
	 * Configuring SPI1
	 */
	SPI1->CR1 &= ~SPI_CR1_BR;		// Baud rate register -> 000 -> CLOCK_F/2
	SPI1->CR1 &= ~SPI_CR1_CPOL;		// Clock polarity -> 0 -> 0 when idle
	SPI1->CR1 &= ~SPI_CR1_CPHA;		// Clock phase -> 0 -> first clock is first data capture
	SPI1->CR1 &= ~SPI_CR1_BIDIMODE;	// Bidirectional data mode -> 0 -> Enabled
	SPI1->CR1 &= ~SPI_CR1_LSBFIRST;	// Frame format -> 0 -> MSB first
	SPI1->CR1 &= ~SPI_CR1_CRCEN;	// CRC -> 0 -> Disabled
	SPI1->CR1 &= ~SPI_CR1_SSM;		// Software Slave Management -> 0
	SPI1->CR1 |=  SPI_CR1_MSTR;		// Master config -> 1 -> Master device
	SPI1->CR2 |=  SPI_CR2_DS_0 |	// Data length -> 0111 -> 8-bit
				  SPI_CR2_DS_1 |
				  SPI_CR2_DS_2;
	SPI1->CR2 &= ~SPI_CR2_SSOE;		// Slave select output enable -> 0
	SPI1->CR2 &= ~SPI_CR2_FRF;		// Frame format -> 0 -> Motarola mode
	SPI1->CR2 &= ~SPI_CR2_NSSP;		// NSS pulse management -> 0 -> No pulse
	SPI1->CR2 &= ~SPI_CR2_FRXTH;	// FIFO reception threshold -> not sure about this one
}
