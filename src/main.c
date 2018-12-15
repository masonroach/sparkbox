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
	
 	lcdTest();

	if (!sdTest()) {
		ledError(1);
		WAV_test();
	}

//	buttonTest();
	
	while(1);
	return 1;
}

void systemInit(void) {
	int8_t i = 0, e = 0;
	
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
	initButtons();
	WAV_Init();
	initLcd();
	initVideo();

	FATFS_LinkDriver(&SD_Driver, SDPath);
    f_mount(&SDFatFs, (TCHAR const*)SDPath, 0);
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
	}
	ledAllOff();
	ledError(0);
	while (readButton());
}

void lcdTest(void) {
	uint8_t leds = 0x00;
	sprite playerSprite, topSprite;

	if (initSprite(&topSprite, "colorTest.spr")){
		// ERROR
		ledError(LED_ERROR);
	}

	if (initSprite(&playerSprite, "dog.spr")){
		// ERROR
		ledError(LED_ERROR);
	}

	drawSpriteDebug(&playerSprite);

	while (!readButton());
	delayms(50);
	while (readButton());

	// Test getting a row for video
	topSprite.ypos = (LCD_HEIGHT - topSprite.height)/2;
	topSprite.xpos = (LCD_WIDTH - topSprite.width)/2;
	playerSprite.xpos = 220;
	playerSprite.ypos = 170;
	spriteLayersAdd(&topSprite);
	spriteLayersAdd(&playerSprite);
	delayms(1000);

	playerSprite.curFrame = 0;

	// Test frame updating with DMA
	while (!readButton()) {
		playerSprite.xvelocity = (BUTTON_RIGHT - BUTTON_LEFT)*5;
		playerSprite.yvelocity = (BUTTON_DOWN - BUTTON_UP)*5;
		topSprite.xvelocity = (BUTTON_B - BUTTON_A)*5;
		topSprite.yvelocity = (BUTTON_X - BUTTON_Y)*5;
		updateFrame();
		ledMap(leds++);
		delayms(5);
	}

	delayms(50);
	while (readButton());
	ledAllOff();

	return;
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

void WAV_test(void)
{
	char testFile[] = "sinewave.wav"; 
	WAV_Format* WAV;

	WAV = (WAV_Format*)malloc(sizeof(WAV_Format));
	if (WAV == NULL) {
		ledOn(0);
		return;
	}

	WAV_Import(testFile, WAV);
	if (WAV->Error != 0) {
		ledOn(1);
		return;
	}

	WAV_Play(WAV, -1);
	if (WAV->Error != 0) {
		ledOn(4);
		return;
	}
	
	// Wait for button to be pressed
	while (readButton());
	while (!readButton());
	WAV_Pause();

	// Don't want to free memory the DMA is transferring
	// WAV_Destroy(WAV);

	return;
	
}
