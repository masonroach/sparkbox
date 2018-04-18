#include "sd.h"

static void csHigh(void) {
	GPIOB->BSRR |= GPIO_BSRR_BS_6;		// Pull Chip Select high
}

static void csLow(void) {
	GPIOB->BSRR |= GPIO_BSRR_BR_6;		// Pull Chip Select low
}

void sdInit(void) {
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
					   GPIO_MODER_MODER5_1 |
					   GPIO_MODER_MODER6_0;		// GPIOB6 -> Output
	GPIOB->AFR[0]  |= (5 << GPIO_AFRL_AFRL3_Pos) |	// Alternate function -> 5
					  (5 << GPIO_AFRL_AFRL4_Pos) |
					  (5 << GPIO_AFRL_AFRL5_Pos);
	GPIOB->OSPEEDR |=  GPIO_OSPEEDER_OSPEEDR3_0 |	// GPIO speed -> Medium
					   GPIO_OSPEEDER_OSPEEDR4_0 |
					   GPIO_OSPEEDER_OSPEEDR5_0;
					   GPIO_OSPEEDER_OSPEEDR6_0;
	GPIOB->OTYPER  &= ~GPIO_OTYPER_OT_6;		// GPIOB6 -> 
	/*
	 * Configuring SPI1
	 */
	SPI1->CR1   &= ~SPI_CR1_BR;			// Baud rate -> 000 -> CLOCK_F/2
	SPI1->CR1   &= ~SPI_CR1_CPOL;		// Clock polarity -> 0 -> 0 when idle
	SPI1->CR1   &= ~SPI_CR1_CPHA;		// Phase -> 0 -> first clock, first data
	SPI1->CR1   &= ~SPI_CR1_BIDIMODE;	// Bidirectional data -> 0 -> Enabled
	SPI1->CR1   &= ~SPI_CR1_LSBFIRST;	// Frame format -> 0 -> MSB first
	SPI1->CR1   |=  SPI_CR1_CRCEN;		// CRC -> 1 -> Enabled
	SPI1->CR1   &= ~SPI_CR1_SSM;		// Software Slave Management -> 0
	SPI1->CR1   |=  SPI_CR1_MSTR;		// Master config -> 1 -> Master device
	SPI1->CR2   |=  SPI_CR2_DS_0 |		// Data length -> 0111 -> 8-bit
					SPI_CR2_DS_1 |
					SPI_CR2_DS_2;
	SPI1->CR2   &= ~SPI_CR2_SSOE;		// Slave select output enable -> 0
	SPI1->CR2   &= ~SPI_CR2_FRF;		// Frame format -> 0 -> Motarola mode
	SPI1->CR2   &= ~SPI_CR2_NSSP;		// NSS pulse management -> 0 -> No pulse
	SPI1->CR2   &= ~SPI_CR2_FRXTH;		// FIFO reception threshold -> not sure rly
	SPI1->CRCPR  = 0x0089;				// CRC poly = x^7 + x^3 + 1
}

void sdSendCmd(SDCOMMAND cmd, uint32_t args) {
	uint8_t i;
	uint8_t frame[5];

	frame[0] = cmd;
	frame[1] = (uint8_t)(args >> (8 * 3));
	frame[2] = (uint8_t)(args >> (8 * 2));
	frame[3] = (uint8_t)(args >> (8 * 1));
	frame[4] = (uint8_t)args;

	while (SPI1->SR & SPI_SR_BSY);	// Wait until spi is not busy
	csLow();						// Pull chip select low
	for (i = 0; i < 5; i++) {		// Send each byte of data
		sdSendByte(frame[i]);
	}
	SPI1->CR1 |= SPI_CR1_CRCNEXT;	// Transmit the CRC
	csHigh();						// Let chip select go high
}

void sdSendByte(uint8_t byte) {
	// Do we need to wait until transmit buffer is empty?
	while (!(SPI1->SR & SPI_SR_TXE));
	SPI1->DR = byte;
}

/*!
 * @breif	Returns the transmit or receive CRC register value
 * @param	crcType: specifies the type of register to be read
 *	This parameter must be one of the following values:
 *		@arg SPI_CRC_RX: Selects the RX CRC register
 *		@arg SPI_CRC_TX: Selects the TX CRC register
 * @retval	The respective CRC value
 */
uint16_t sdGetCRC(CRCTYPE crcType) {
	if (crcType == SPI_CRC_RX) {
		return SPI1->RXCRCR;
	} else {	// Then it must be SPI_CRC_TX
		return SPI1->TXCRCR;
	}
}
