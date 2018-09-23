#include "main.h"

void systemInit(void);
void lcdTest(void);
void sdTest(void);

int main(void) {
	uint16_t i = 0;

	// Initialize peripherals
	systemInit();
	
	// Main code
	sdTest();

	while (1);

	return 0;
	
}

void systemInit(void) {
	int8_t i = 0;
	int8_t e = 0;
	volatile uint32_t j = 0;

	// Init system clock first
	if (ledMap(initSystemClock())) ledError(2);

//	HAL_Init();
	initLeds();
	initButton();
	initLcd();

	ledError(LED_OFF);
	while (readButton()) {
		i > 8 ? i = 0 : i++;
		ledMap((0xFF >> (8-i)) & 0xFF);
//		ledMap(0xFF & rand32());
		ledError(e > 2 ? e = 0 : e++);
		delayms(100);
//		for (j = 0; j < 500000; j++);
	}
	while (readButton() == 1);
	ledAllOff();
	ledError(LED_OFF);
}

void lcdTest(void) {
//	LcdWriteCmd(0xFFFF);
	while (1) {
//		LcdWriteCmd(i++);
		LcdWriteCmd(1);
		LcdWriteCmd(0);
		LcdWriteCmd(1);
		LcdWriteCmd(0);
		LcdWriteCmd(1);
		while (readButton());
		while (!readButton());
	}
}

void sdTest(void) {
	FRESULT res;                                          /* FatFs function common result code */
 	uint32_t byteswritten, bytesread;                     /* File write/read counts */
 	uint8_t wtext[] = "Sparkbox's first file!"; /* File write buffer */
 	TCHAR fileName[] = "sparkbox.txt"; 				  /* File name */
	uint8_t rtext[100];
	volatile int i;
	FATFS SDFatFs;  /* File system object for SD card logical drive */
	FIL MyFile;     /* File object */
	char SDPath[4]; /* SD card logical drive path */

	if(FATFS_LinkDriver(&SD_Driver, SDPath) != 0) {goto end;}
	if(f_mount(&SDFatFs, (TCHAR const*)SDPath, 0) != FR_OK) {goto end;}
	if(f_open(&MyFile, fileName, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) {goto end;}
	if(f_write(&MyFile, wtext, sizeof(wtext), (void *)&byteswritten) != FR_OK) {
		f_close(&MyFile); 
		goto end; 
	}
	f_close(&MyFile);
	if(f_open(&MyFile, fileName, FA_READ) != FR_OK) {goto end;}
    if(f_read(&MyFile, rtext, sizeof(rtext), (UINT*)&bytesread) != FR_OK) {
		f_close(&MyFile);
		goto end;
	}
	f_close(&MyFile);
	if((bytesread != byteswritten)){goto end;}


	
	// SUCCESS, let's play a WAV file
	ledError(1);

	// PLAY WAV FILE HERE

	goto end2;

end:
	ledError(2);
end2:
	FATFS_UnLinkDriver(SDPath);
	// Push button test
	while(1);

}
