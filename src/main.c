#include "stm32f3xx.h"
#include "usart.h"
#include "led.h"
#include "button.h"
#include "sd.h"

int main(void) {
	int8_t i = 0;

	initLeds();
	usartConfig();
	initButton();	
	sdInit();

	sdSendCmd(0xFF, 0x00000000);

	while (1) {
		while (readButton() == 0);	// Wait while button is not pushed
		if (++i > 8) i = 0;
		ledCircle(i);
		while (readButton() == 1);	// Wait while button is pushed
		usartSendChar('0' + readButton());
	}

	return 1;
}
