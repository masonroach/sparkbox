#include "usart.h"

// Configure 8N1 USART over USART1 port
void initUsart(void) {

	/*
	 * Initialize GPIOs
	 */
	RCC->AHB1ENR   |=   RCC_AHB1ENR_GPIOAEN;	// Enable GPIOA Clock

	// PA9 Config (TX)
	GPIOA->MODER   |=   GPIO_MODER_MODER9_1;	// GPIO Mode AF
	GPIOA->AFR[1]  |=   GPIO_AFRH_AFSEL9 & (7 << GPIO_AFRH_AFSEL9_Pos);		// Alternate function 7
	GPIOA->OTYPER  |=   GPIO_OTYPER_OT_9;		// GPIO OType OD
	GPIOA->OSPEEDR |=   GPIO_OSPEEDER_OSPEEDR9;	// GPIO Speed 50MHz
	GPIOA->PUPDR   &=  ~GPIO_PUPDR_PUPDR9;		// GPIO No Pull

	// PA10 Config (RX)
	GPIOA->MODER   |=   GPIO_MODER_MODER10_1;	// GPIO Mode AF
	GPIOA->AFR[1]  |=   GPIO_AFRH_AFSEL10 & (7 << GPIO_AFRH_AFSEL10_Pos);	// Alternate function 7

	/*
	 * Initialize USART
	 */	
	RCC->APB2ENR |=  RCC_APB2ENR_USART1EN;	// Enable USART Clock

#if OVER8
	// Oversampling mode 8
	USART1->CR1  |=  USART_CR1_OVER8;	// Set oversampling mode = 8
	USART1->BRR   = (USARTDIV & 0xFFF0) | ((USARTDIV & 0x000F) >> 1);	// Set USART divider
#else
	// Oversampling mode 16
	USART1->CR1  &= ~USART_CR1_OVER8;	// Set oversampling mode = 16
	USART1->BRR   =  USARTDIV & 0xFFFF;	// Set USART divider
#endif

	USART1->CR1  &= ~USART_CR1_M;		// M[1:0] = 8 bit word length
	USART1->CR1  &= ~USART_CR1_PCE;		// No Parity
	USART1->CR1  |=  USART_CR1_TE;		// Transmitter enable
	USART1->CR1  |=  USART_CR1_RE;		// Receiver enable
	USART1->CR1  |=  USART_CR1_UE;		// USART enable
	USART1->CR2  &= ~USART_CR2_STOP;	// One stop bit
}

// Send a character over USART
void usartSendChar(int8_t c) {
	while (!(USART1->SR & USART_SR_TXE));	// Wait for transmit buffer to be empty
	USART1->DR = c;			// Send the byte
}

// Receive a character over USART
uint8_t usartGetChar(void) {
	while (!(USART1->SR & USART_SR_RXNE));	// Wait for receive buffer to be full
	return USART1->DR;		// Get the byte
}

// Send a string over USART
void usartSendString(int8_t *s) {
	while (*s != '\0') usartSendChar(*(s++));
}

void usartSendHex(uint8_t byte) {
	uint8_t byteH, byteL;

	byteH = (byte/16) > 9 ? (byte/16) - 10 + 'A' : (byte/16) + '0';
	byteL = (byte%16) > 9 ? (byte%16) - 10 + 'A' : (byte%16) + '0';
	usartSendChar(byteH);
	usartSendChar(byteL);
}
