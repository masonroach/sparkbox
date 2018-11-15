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
		ledError(2);
		WAV_test();
	}
	buttonTest();
	
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
	while (1/*!readButton()*/) {
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
	uint8_t *testString = (uint8_t *)"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	uint16_t temp;
	sprite testSprite;
	sprite *tSprite;

	// Test lcd coordinates/orientation
	LcdPutPixel(20, 20, LCD_COLOR_RED);
	LcdDrawRectangle(0, 0, 10, 10, LCD_COLOR_RED);
	LcdPutPixel(299, 20, LCD_COLOR_BLUE);
	LcdDrawRectangle(310, 0, 10, 10, LCD_COLOR_BLUE);
	LcdPutPixel(20, 219, LCD_COLOR_GREEN);
	LcdDrawRectangle(0, 229, 10, 10, LCD_COLOR_GREEN);

	while (!readButton());
	delayms(50);
	while (readButton());

	// Basic screen drawing tests
	LcdFillScreen(LCD_COLOR_BLACK);
	LcdDrawRectangle((LCD_WIDTH-70)/2, (LCD_HEIGHT-70)/2, 70, 70, LCD_COLOR_WHITE);
	LcdDrawRectangle(10, 200, 300, 1, LCD_COLOR_WHITE);
	LcdDrawString(20, 10, testString, LCD_COLOR_WHITE, LCD_COLOR_BLACK);
	LcdDrawString(140, 130, (uint8_t *)"SPARKBOX", LCD_COLOR_BLACK, LCD_COLOR_WHITE);
	LcdDrawInt(20, 120, 256, LCD_COLOR_WHITE, LCD_COLOR_BLACK);
	LcdDrawInt(20, 180, 10, LCD_COLOR_WHITE, LCD_COLOR_BLACK);
	LcdDrawHex(20, 150, 0xDEAD, LCD_COLOR_WHITE, LCD_COLOR_BLACK);

	while (!readButton());
	delayms(50);
	while (readButton())

	// Test reading a pixel
	LcdFillScreen(0xFEED);
	temp = LcdReadPixel(50, 50);
	LcdDrawHex(10, 10, temp, LCD_COLOR_WHITE, LCD_COLOR_BLACK);
	

	while (!readButton());
	delayms(50);
	while (readButton())
	delayms(1000);

	// Test sprites
	if (test_getSprite(&testSprite)) {
		// ERROR
		ledError(LED_ERROR);
	}
	drawSprite(&testSprite);

	while (!readButton());
	delayms(50);
	while (readButton())
	delayms(1000);
/*
	test_fseek(0, TEST_SEEK_SET);
	test_drawSprite();

	while (!readButton());
	delayms(50);
	while (readButton())
	delayms(1000);
*/
	// Test filling screen
	while (!readButton()) {
		LcdFillScreen(LCD_COLOR_BLUE);
		LcdFillScreen(LCD_COLOR_RED);
		LcdFillScreen(LCD_COLOR_GREEN);
	}

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

void WAV_test(void)
{
	char testFile[20] = "youGotMail.wav"; 
	WAV_Format* WAV;

	WAV = (WAV_Format*)malloc(sizeof(WAV_Format));
	if (WAV == NULL) {
		ledOn(0);
		return;
	}

	WAV_Import(testFile, WAV);
	if (WAV->Error != 0) {
		ledOn(2);
		return;
	}

	WAV_Play(WAV, -1);
	if (WAV->Error != 0) {
		ledOn(4);
		return;
	}
	

	// Don't want to free memory the DMA is transferring
	// WAV_Destroy(WAV);

	return;
	
}
