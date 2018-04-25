#include "stm32f3xx.h"
//#include "clock.h"
#include "usart.h"
#include "led.h"
#include "button.h"
#include "ff.h"
#include "ff_gen_drv.h"
#include "sd_diskio.h"

void systemInit(void);

FATFS SDFatFs;
FIL MyFile;
char SDPath[4];

int main(void) {
	uint8_t i = 0;
	FRESULT res;
	uint32_t byteswritten, bytesread;
	uint8_t rtext[100];

	systemInit();

	if (FATFS_LinkDriver(&SD_Driver, SDPath) == 0) {
		ledOn(1);
		usartSendString("FATFS_LinkDriver() Worked");
	}

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
