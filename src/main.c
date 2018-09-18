#include "main.h"

void systemInit(void);

int main(void) {
	uint16_t i = 0;

	// Initialize peripherals
	systemInit();
	
	// Main code
//	LcdWriteCmd(0xFFFF);
	while (1) {
		LcdWriteCmd(i++);
	}

	// End with a dead loop
	while (1);
	return 0;
	
}

void systemInit(void) {
	int8_t i = 0;
	int8_t e = 0;
	volatile uint32_t j = 0;
	
	initLeds();
	initButton();
	initLcd();
	initRng();
	
	/*
	 * Initialization is complete. User can press the button to continue at
	 * any time. Until then, a single serial message will be sent, and the
	 * LEDs will continue to light up in a circle.
	 */
	while (readButton()) {
		i > 8 ? i = 0 : i++;
//		ledMap((0xFF >> (8-i)) & 0xFF);
		ledMap(0xFF & rand32());
		ledError(e > 2 ? e = 0 : e++);
		for (j = 0; j < 300000; j++);
	}
	ledAllOff();
	ledError(LED_OFF);
}
