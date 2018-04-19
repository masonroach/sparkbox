#include "usart.h"

// Configure 8N1 USART over USART1 port
void usartConfig(void) {

	/*
	 * Initialize GPIOs
	 */
	RCC->AHBENR    |=   RCC_AHBENR_GPIOCEN;	// Enable GPIOC Clock

	// PC4 Config (TX)
	GPIOC->MODER   |=   GPIO_MODER_MODER4_1;	// GPIO Mode AF
	GPIOC->AFR[0]  |=   GPIO_AFRL_AFRL4 & (7 << GPIO_AFRL_AFRL4_Pos);	// Alternate function 7
	GPIOC->OTYPER  |=   GPIO_OTYPER_OT_4;		// GPIO OType OD
	GPIOC->OSPEEDR |=   GPIO_OSPEEDER_OSPEEDR4;	// GPIO Speed 50MHz
	GPIOC->PUPDR   &=  ~GPIO_PUPDR_PUPDR4;		// GPIO No Pull

	// PC5 Config (RX)
	GPIOC->MODER   |=   GPIO_MODER_MODER5_1;	// GPIO Mode AF
	GPIOC->AFR[0]  |=   GPIO_AFRL_AFRL5 & (7 << GPIO_AFRL_AFRL5_Pos);	// Alternate function 7

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
	while (!(USART1->ISR & USART_ISR_TXE));	// Wait for transmit buffer to be empty
	USART1->TDR = c;			// Send the byte
}

// Receive a character over USART
uint8_t usartGetChar(void) {
	while (!(USART1->ISR & USART_ISR_RXNE));	// Wait for receive buffer to be full
	return USART1->RDR;
}

// Send a string over USART
void usartSendString(int8_t *s) {
	while (*s != '\0') usartSendChar(*(s++));
}

void usartSendByte(uint8_t byte) {
	uint8_t byteH, byteL;

	byteH = (byte/16) > 9 ? (byte/16) - 10 + 'A' : (byte/16) + '0';
	byteL = (byte%16) > 9 ? (byte%16) - 10 + 'A' : (byte%16) + '0';
	usartSendChar(byteH);
	usartSendChar(byteL);
}
