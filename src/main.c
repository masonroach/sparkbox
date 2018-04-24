#include "stm32f3xx.h"
#include "usart.h"
#include "led.h"
#include "button.h"
#include "sd.h"

void systemInit(void);
void testSdSendByte(uint8_t byte);
void checkCRC(void);

int main(void) {
	uint8_t i = 0;
	uint8_t crc, crcH, crcL;

	systemInit();

	GPIOB->BSRR |= GPIO_BSRR_BR_6;		// CS low
/*	usartSendString("Received: ");
	usartSendByte(sdGetByte());
	usartSendString("\r\n");
*/
	sdCardInit();
	usartSendByte(sdReadByte());
	usartSendByte(sdReadByte());
	usartSendByte(sdReadByte());
	usartSendByte(sdReadByte());
	usartSendByte(sdReadByte());
	usartSendByte(sdReadByte());
	sdCardInit();
	usartSendByte(sdReadByte());
	usartSendByte(sdReadByte());
	usartSendByte(sdReadByte());
	usartSendByte(sdReadByte());
	usartSendByte(sdReadByte());
	usartSendByte(sdReadByte());
	usartSendByte(sdReadByte());
	usartSendByte(sdReadByte());
//	testSdSendByte(0x95);
	GPIOB->BSRR |= GPIO_BSRR_BS_6;		// CS high	
	
//	sdSendCmd(GO_IDLE_STATE, 0);

	while (1) {
		while (readButton() == 0);	// Wait while button is not pushed
		if (++i > 8) i = 0;
		ledCircle(i);
		while (readButton() == 1);	// Wait while button is pushed
		usartSendChar('0' + readButton());
	}

	return 1;
}

void systemInit(void) {
	int8_t i = 0;
	volatile uint16_t j = 0;

	initLeds();
	usartConfig();
	initButton();	
	sdSpiInit();
	HAL_Init();
	
	/*
	 * Initialization is complete. User can press the button to continue at
	 * any time. Until then, a single serial message will be sent, and the
	 * LEDs will continue to light up in a circle.
	 */
	usartSendString("Initialized. Press button to continue.\r\n");
	while (readButton() == 0) {
		if (++i > 15) i = 0;
		if (i <= 8) {
			ledCircle(i);
		} else {
			ledCircleInverted(i%8);
		}
		for (j = 0; j < 50000; j++);
	}
	ledAllOff();
}

void testSdSendByte(uint8_t byte) {

	sdSendByte(byte);
	usartSendString("Sent: 0x");
	usartSendByte(byte);
	usartSendString("\r\n");

}

void checkCRC(void) {
	uint8_t crc;

	crc = (uint8_t)sdGetCRC(SPI_CRC_TX);

	usartSendString("CRC: 0x");
	usartSendByte(crc);
	usartSendString("\r\n");
}
