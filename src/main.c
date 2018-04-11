#include "stm32f3xx.h"
#include "usartDriver.h"
#include "ledDriver.h"
#include "button.h"

int main(void) {
	int8_t i = 0;

	initLeds();
	usartConfig();
	initButton();	

	while (1) {
		while (readButton() == 0);	// Wait while button is not pushed
		if (++i > 8) i = 0;
		ledCircle(i);
		while (readButton() == 1);	// Wait while button is pushed
		usartSendChar('0' + readButton());
	}

	return 1;
}
