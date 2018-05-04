#include "stm32f303xc.h"

#ifndef SPARK_USART
#define SPARK_USART

#ifndef CLOCK_F
#define CLOCK_F 8000000UL
#endif

#define BAUD 9600	// BAUD rate
#define OVER8 1		// Oversampling by 8

#if OVER8		// If oversampling by 8
#define USARTDIV 2*CLOCK_F/BAUD
#else			// If oversampling by 16
#define USARTDIV CLOCK_F/BAUD
#endif

void usartConfig(void);
void usartSendChar(int8_t c);
uint8_t usartGetChar(void);
void usartSendString(int8_t *s);
void usartSendHex(uint8_t byte);

#endif
