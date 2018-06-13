#include "stm32f4xx.h"
//#include "clock.h"	// Include clock before others
//#include "usart.h"
#include "led.h"
#include "button.h"
//#include "sd.h"
// #include "pwm.h"
//#include "pushButton.h"

void systemInit(void);

int main(void) {
	uint8_t i = 0;
	// int freq[4] = {1000, 2000, 3000, 4000}; // Continuous time frequencies
	// unsigned char vol[4] = {100, 100, 100, 100}; // Volumes in terms of percent

	systemInit();

	/*
	initAudio();
	setFrequency(freq);
	setVolume(vol);
	*/

/*
	// Test 1 FATFS_LinkDriver()
	if (FATFS_LinkDriver(&SD_Driver, SDPath) == 0) {
//		ledOn(1);
//		usartSendString("FATFS_LinkDriver() Worked\r\n");
	} else return 1;

	// Test 2 f_mount
	if (f_mount(&SDFatFs, (TCHAR const*)SDPath, 0) != FR_OK) {
		goto end;
	}
//	ledOn(2);
//	usartSendString("f_mount() Worked\r\n");

	// Test 3 mkfs()
	res = f_mkfs((TCHAR const*)SDPath, 0, 0);
	if (res != FR_OK) {
		if (res == FR_INVALID_PARAMETER)
			usartSendString("res == FR_INVALID_PARAMETER\r\n");
		if (res == FR_DISK_ERR)
			usartSendString("res == FR_DISK_ERR\r\n");
		if (res == FR_INVALID_DRIVE)
			usartSendString("res == FR_INVALID_DRIVE\r\n");
		if (res == FR_NOT_READY);
			usartSendString("res == FR_NOT_READY\r\n");
		if (res == FR_WRITE_PROTECTED)
			usartSendString("res == FR_WRITE_PROTECTED\r\n");
		if (res == FR_MKFS_ABORTED)
			usartSendString("res == FR_MKFS_ABORTED\r\n");
		goto end;
	}
	ledOn(3);
	usartSendString("f_mkfs() Worked\r\n");

	// Test 4 f_open
	if (f_open(&MyFile, "Test.txt", FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) {
		goto end;
	}
	ledOn(4);
	usartSendString("f_open() (write) Worked\r\n");

	// Test 5 f_write
	res = f_write(&MyFile, wtext, sizeof(wtext), (void *)&byteswritten);
	if ((byteswritten == 0) || (res != FR_OK)) {
		goto end;
	}
	ledOn(5);
	usartSendString("f_write() Worked\r\n");

	// Test 6 f_close
	if (f_close(&MyFile) != FR_OK) {
		goto end;
	}
	ledOn(6);
	usartSendString("f_close() Worked\r\n");

	// Test 7 f_open
	if (f_open(&MyFile, "Test.txt", FA_READ) != FR_OK) {
		goto end;
	}
	ledOn(7);
	usartSendString("f_open() (read) Worked\r\n");

	// Test 8 f_read
	res = f_read(&MyFile, rtext, sizeof(rtext), (UINT*)&bytesread);
	if ((bytesread == 0) || (res != FR_OK)) {
		goto end;
	}
	ledOn(8);
	usartSendString("f_read() Worked\r\n");

	// Test f_close
	f_close(&MyFile);

	// Test bytesread = byteswritten
	if (bytesread != byteswritten) {
		usartSendString("bytesread != bytes written ");
		usartSendHex((uint8_t)bytesread);
		usartSendChar(' ');
		usartSendHex((uint8_t)bytesread);
		usartSendString("\r\n");
		goto end;
	}
	usartSendString("bytesread == byteswritten");

	FATFS_UnLinkDriver(SDPath);
*/

	while(1){
/*		if (BUTTON_A || BUTTON_B || BUTTON_X || BUTTON_Y ||
			BUTTON_LEFT || BUTTON_RIGHT || BUTTON_UP || BUTTON_DOWN) ledOn(1);
		else ledOff(1);
*/	
		ledOn(GREEN);
	}

	return 0;
/*
end:
	FATFS_UnLinkDriver(SDPath);
	while (1);
	return 1;
*/
}

void systemInit(void) {
	int8_t i = 0;
	volatile uint16_t j = 0;

//	initUsart();
	initLeds();
	initButton();
//	initSdSpi();
	
	/*
	 * Initialization is complete. User can press the button to continue at
	 * any time. Until then, a single serial message will be sent, and the
	 * LEDs will continue to light up in a circle.
	 */
//	usartSendString("Initialized. Press button to continue.\r\n");
	while (readButton() == 0) {
		if (++i > 7) i = 0;
		if (i <= 4) {
			ledCircle(i);
		} else {
			ledCircleInverted(i%4);
		}
		for (j = 0; j < 50000; j++);
	}
	ledAllOff();
}
