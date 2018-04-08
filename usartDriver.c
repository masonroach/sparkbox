#include "usartDriver.h"

// Pulled from cem.itesm.mx
void GPIO_Config(void) {
	// PC4 Config (TX)
	RCC->AHBENR    |=   1 << 19;	// Enable GPIOC Clock
	GPIOC->MODER   |=   2 << (4*2);	// GPIO Mode AF
	GPIOC->OTYPER  |=   1 << (4*1);	// GPIO OType OD
	GPIOC->OSPEEDR |=   3 << (4*2);	// GPIO Speed 50MHz
	GPIOC->PUPDR   &= ~(3 << (4*2));	// GPIO No Pull
	GPIOC->AFR[0]  |=   7 << (4*4);	// AF7

	// PC5 Config (RX)
	GPIOC->MODER   |=   2 << (5*2);	// GPIO Mode AF
	GPIOC->AFR[0]  |=   7 <<(5*4);	// AF7
}

// Configure 8N1 USART over USART1 port
void USART_Config(void) {
	RCC->APB2ENR |=  RCC_APB2ENR_USART1EN;	// Enable USART Clock
#if OVER8
	USART1->CR1  &= ~USART_CR1_OVER8;	// Oversampling mode = 16
	USART1->BRR   =  USARTDIV;		// Set USART divider
#else
	USART1->CR1  |=  USART_CR1_OVER8;	// Oversampling mode = 8
	USART1->BRR   = (USARTDIV & 0xFFF0) | ((USARTDIV & 0x000F) >> 1);	// Set USART divider
#endif
	USART1->CR1  &= ~USART_CR1_M;		// M[1:0] = 8 bit word length
	USART1->CR1  &= ~USART_CR1_PCE;		// No Parity
	USART1->CR1  |=  USART_CR1_TE;		// Transmitter enable
	USART1->CR1  |=  USART_CR1_RE;		// Receiver enable
	USART1->CR1  |=  USART_CR1_UE;		// USART enable
	USART1->CR2  &= ~USART_CR2_STOP;	// One stop bit
}

// Send a character over USART
void usartSendChar(uint8_t c) {
	while (!(USART1->ISR & USART_ISR_TXE));	// Wait for transmit buffer to be empty
	USART1->TDR = c;			// Send the byte
}

// Receive a character over USART
uint8_t usartGetChar(void) {
	while (!(USART1->ISR & USART_ISR_RXNE));	// Wait for receive buffer to be full
	return USART1->RDR;
}

// Send a string over USART
void usartSendString(uint8_t *s) {
	while (*s != '\0') usartSendChar(*(s++));
}
