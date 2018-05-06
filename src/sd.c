#include "sd.h"

void csHigh(void) {
	GPIOA->BSRR |= GPIO_BSRR_BS_4;		// Pull Chip Select high
}

void csLow(void) {
	GPIOA->BSRR |= GPIO_BSRR_BR_4;		// Pull Chip Select low
}

/*!
 * @brief	Initializes pin connections to SD card
 *		PA3: CD
 *		PA4: CS
 *		PA5: SCLK
 *		PA6: MISO
 *		PA7: MOSI
 * @param	None
 * @retval	None
 */
void sdSpiInit(void) {

	/*
	 * Enabling clocks
	 */
	RCC->AHBENR   |=  RCC_AHBENR_GPIOBEN |	// Enable GPIOB and GPIOA clock
					  RCC_AHBENR_GPIOAEN;
	RCC->APB2ENR  |=  RCC_APB2ENR_SPI1EN;	// Enable SPI1 clock
	RCC->APB2RSTR |=  RCC_APB2RSTR_SPI1RST;	// Reset SPI1
	RCC->APB2RSTR &= ~RCC_APB2RSTR_SPI1RST;	// Clear reset of SPI1

	/*
	 * Configure SPI GPIOs
	 */
	GPIOA->MODER &= ~GPIO_MODER_MODER3;		// PA3 (CD) Mode -> Input
	GPIOA->MODER |=  GPIO_MODER_MODER4_0 |	// PA4 (CS) Mode -> Output
					 GPIO_MODER_MODER5_1 |	// PA5 (SCLK) Mode -> AF
					 GPIO_MODER_MODER6_1 |	// PA6 (MISO) Mode -> AF
					 GPIO_MODER_MODER7_1;	// PA6 (MOSI) Mode -> AF
	GPIOA->AFR[0] |= (5 << GPIO_AFRL_AFRL5_Pos) |	// PA5 AF 5 -> SCLK
					 (5 << GPIO_AFRL_AFRL6_Pos) |	// PA6 AF 5 -> MISO
					 (5 << GPIO_AFRL_AFRL7_Pos);	// PA7 AF 5 -> MOSI
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR3_0 |	// PA3 (CD) -> Pull-up resistor
					GPIO_PUPDR_PUPDR6_0;	// PA6 (MISO) -> Pull-up resistor

	/*
	 * Configuring SPI1
	 */
	SPI1->CR1   &= ~SPI_CR1_BR;			// Baud rate -> 100 -> CLOCK_F/32
	SPI1->CR1   |=  SPI_CR1_BR_2;		
	SPI1->CR1   &= ~SPI_CR1_CPOL;		// Clock polarity -> 0 -> 0 when idle
	SPI1->CR1   &= ~SPI_CR1_CPHA;		// Phase -> 0 -> first clock, first data
	SPI1->CR1   &= ~SPI_CR1_RXONLY;		// Receive only -> 0 -> Full duplex
	SPI1->CR1   &= ~SPI_CR1_BIDIMODE;	// Bidirectional data -> 0 -> Enabled
	SPI1->CR1   |=  SPI_CR1_BIDIOE;		// Bidirectional output enable -> 1
	SPI1->CR1   &= ~SPI_CR1_LSBFIRST;	// Frame format -> 0 -> MSB first
	SPI1->CR1   &= ~SPI_CR1_CRCEN;		// CRC -> 0 -> Disabled
	SPI1->CR1   &= ~SPI_CR1_CRCL;		// CRC length -> 0 -> 8 bits
	SPI1->CR1   |=  SPI_CR1_SSM;		// Slave Management -> 1 -> Software
	SPI1->CR1   |=  SPI_CR1_SSI;		// Because Yifeng's book told me to
	SPI1->CR1   |=  SPI_CR1_MSTR;		// Master config -> 1 -> Master device
	SPI1->CR2   |=  SPI_CR2_DS_0 |		// Data length -> 0111 -> 8-bit
					SPI_CR2_DS_1 |
					SPI_CR2_DS_2;
	SPI1->CR2   |=  SPI_CR2_SSOE;		// Slave select output enable -> 1
	SPI1->CR2   &= ~SPI_CR2_FRF;		// Frame format -> 0 -> Motarola mode
	SPI1->CR2   &= ~SPI_CR2_NSSP;		// NSS pulse management -> 0 -> No pulse
	SPI1->CR2   |=  SPI_CR2_FRXTH;		// FIFO reception threshold -> 8 - bit
	SPI1->CR1   |=  SPI_CR1_SPE;		// SPI Enable -> 1 -> Enabled
//	SPI1->CRCPR  = 0x0089;				// CRC poly = x^7 + x^3 + 1

	csHigh();
	// **********DELAY 10MS***********
}

void sdCardInit(void) {
	uint8_t i;

	/*
	 * Start initializing the SD card over SPI
	 */
	for (i = 0; i < 9; i++) {
		sdSendByte(0xFF);
		usartSendChar(i+'0');
	}
	sdSendCmd(GO_IDLE_STATE, 0x00000000UL);
}

void sdSendCmd(SDCOMMAND cmd, uint32_t args) {
	uint8_t i;
	uint8_t frame[6];

	// Split commands into byte-size frames
	frame[0] = (cmd | 0x40);
	frame[1] = (uint8_t)(args >> (8 * 3));
	frame[2] = (uint8_t)(args >> (8 * 2));
	frame[3] = (uint8_t)(args >> (8 * 1));
	frame[4] = (uint8_t)args;

	// Determine if CRC is needed
	if (cmd == GO_IDLE_STATE) frame[6] = 0x95;			// CRC for CMD0
	else if (cmd == SEND_IF_COND) frame[6] = 0x87;		// CRC for CMD8
	else frame[6] = 0x01;								// Dummy CRC + stop bit

	csLow();							// Pull chip select low
	for (i = 0; i < 6; i++) {			// Send each byte of data
		sdSendByte(frame[i]);
		usartSendHex(frame[i]);
		usartSendChar('\n');
	}

	csHigh();							// Let chip select go high
}

uint8_t sdSendByte(uint8_t byte) {
	while (!(SPI1->SR & SPI_SR_TXE)); 	// Wait until transmit buffer is empty
	SPI1->DR = byte;					// Send the byte
	while (SPI1->SR & SPI_SR_BSY);		// Wait until spi is not busy

	return (uint8_t)SPI1->DR;
}

/*!
 * @brief	Returns the transmit or receive CRC register value
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

uint8_t sdReadByte(void) {
	uint8_t data;
	sdSendByte(0xFF);					// Send a dummy byte
	while (SPI1->SR & SPI_SR_RXNE);		// Wait until receive buffer not empty
	data = (uint8_t)SPI1->DR;
	return data;
}
