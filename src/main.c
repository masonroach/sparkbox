#include "main.h"

/* Private function prototypes -----------------------------------------------*/
void systemInit(void);
void lcdTest(void);
uint8_t sdTest(void);
void WAV_test(void);
void buttonTest(void);

FATFS SDFatFs;  /* File system object for SD card logical drive */
char SDPath[4]; /* SD card logical drive path */
FIL MyFile;     /* File object */

int main(void) {
	
	systemInit();
	
	if (!sdTest()) {
		ledError(2);
		WAV_test();
	}
	buttonTest();

	
	while(1);
	return 1;
}

void systemInit(void) {
	int8_t i = 0, e = 0;
	volatile uint32_t j = 0;
	
	/* STM32F4xx HAL library initialization:
       - Configure the Flash prefetch, instruction and Data caches
       - Configure the Systick to generate an interrupt each 1 msec
       - Set NVIC Group Priority to 4
       - Global MSP (MCU Support Package) initialization
     */
	HAL_Init();
  
	/* Configure the system clock to 168 MHz */
	initSystemClock();

//	initUsart();
	initLeds();
	initButton();
	initButtons();
	initLcd();
	WAV_Init();
//	initSdSpi();
	// RCC->AHB1ENR    |=   RCC_AHB1ENR_GPIODEN;
	// RCC->AHB1ENR    |=   RCC_AHB1ENR_GPIOCEN;
	

	
	/*
	 * Initialization is complete. User can press the button to continue at
	 * any time. Until then, a single serial message will be sent, and the
	 * LEDs will continue to light up in a circle.
	 */
//	usartSendString("Initialized. Press button to continue.\r\n");
	while (!readButton()) {
		i > 8 ? i = 0 : i++;
		ledMap((0xFF >> (8-i)) & 0xFF);
//		ledMap(0xFF & rand32());
		ledError(e > 2 ? e = 0 : e++);
		delayms(100);
		//for (j = 0; j < 500000; j++);
	}
	ledAllOff();
	ledError(0);
	while (readButton());
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

void buttonTest(void)
{
	ledAllOff();
	while (1) {
		if (BUTTON_LEFT) ledOn(0);
		else ledOff(0);

		if (BUTTON_RIGHT) ledOn(1);
		else ledOff(1);

		if (BUTTON_UP) ledOn(2);
		else ledOff(2);

		if (BUTTON_DOWN) ledOn(3);
		else ledOff(3);

		if (BUTTON_A) ledOn(4);
		else ledOff(4);

		if (BUTTON_B) ledOn(5);
		else ledOff(5);

		if (BUTTON_X) ledOn(6);
		else ledOff(6);

		if (BUTTON_Y) ledOn(7);
		else ledOff(7);
	}
}

uint8_t sdTest(void) {
 	uint32_t byteswritten, bytesread;                     /* File write/read counts */
 	uint8_t wtext[] = "Sparkbox's first file!"; /* File write buffer */
 	TCHAR fileName[] = "sparkbox.txt"; 				  /* File name */
	uint8_t rtext[100];
	volatile int i;

	if (FATFS_LinkDriver(&SD_Driver, SDPath) != 0) {goto end;}
	if (f_mount(&SDFatFs, (TCHAR const*)SDPath, 0) != FR_OK) {goto end;}
	if (f_open(&MyFile, fileName, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) {goto end;}
	if (f_write(&MyFile, wtext, sizeof(wtext), (void *)&byteswritten) != FR_OK) {
		f_close(&MyFile); 
		goto end; 
	}
	f_close(&MyFile);
	if (f_open(&MyFile, fileName, FA_READ) != FR_OK) {goto end;}
    if (f_read(&MyFile, rtext, sizeof(rtext), (UINT*)&bytesread) != FR_OK) {
		f_close(&MyFile);
		goto end;
	}
	f_close(&MyFile);
	if ((bytesread != byteswritten)){goto end;}


	
	// SUCCESS
	return 0;
	
end:
	// FAIL
	FATFS_UnLinkDriver(SDPath);
	return -1;
}

#define BUFFER_BYTE 19802
#define BUFFER_WORD ((BUFFER_BYTE / 4)+1)
void WAV_test(void)
{
	uint8_t i=0;
	char testFile[14] = "test.wav"; 
	uint32_t* wavBuffer;
	WAV_Format* WAV;

	WAV = (WAV_Format*)malloc(sizeof(WAV_Format));
	if (WAV == NULL) {
		ledOn(0);
		return;
	}

	wavBuffer = (uint32_t*)malloc(sizeof(uint32_t) * BUFFER_WORD);
	if (wavBuffer == NULL) {
		ledOn(1);
		return;
	}


	WAV_Import(testFile, WAV, wavBuffer);
	if (WAV->Error != 0) {
		ledOn(2);
		return;
	}

	WAV_Play((uint32_t)wavBuffer, WAV, -1);
	if (WAV->Error != 0) {
		ledOn(4);
		return;
	}
	

	// Don't want to free memory the DMA is transferring
	// free(WAV);
	// free(wavBuffer);

	return;
	
}
