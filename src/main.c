#include "main.h"

/* Private function prototypes -----------------------------------------------*/
void systemInit(void);
void lcdTest(void);
uint8_t sdTest(void);
void WAV_test(void);
void buttonTest(void);
void playGame(void);

FATFS SDFatFs;  /* File system object for SD card logical drive */
char SDPath[4]; /* SD card logical drive path */
FIL MyFile;     /* File object */
volatile sprite dog, rain1, rain2;

int main(void) {
	
	systemInit();
	

	WAV_test();

 	lcdTest();

	WAV_Pause();

	playGame();
	
	while(1);
	return 1;
}


void playGame(void)
{
	uint32_t rand = 87;
	uint32_t done = 0;
	uint16_t x, y;
	uint32_t score = 0;
	WAV_Format* WAV;

	// Allocate memory for wav file
	WAV = (WAV_Format*)malloc(sizeof(WAV_Format));
	WAV_Import("fused.wav", WAV);
	if (WAV->Error != 0) {
		for (x = 0; x < WAV->Error; x++) {
			ledError(2);
			delayms(200);
			ledError(0);
			delayms(200);
		}
	}


	dog.xpos = 30;
	dog.ypos = 100;

	rain1.xpos = 220;
	rain1.ypos = 110;

	rain2.xpos = 200;
	rain2.ypos = 150;

	rain1.xvelocity = -5;
	rain2.xvelocity = -5;
	frameUpdateOn();

	while(!done) {
		// Reset rainbows
		if (rain1.xpos < rain1.xvelocity) {
			rand = (50021 * rand + 50023) % 50051;
			rain1.ypos = rand % 160 + 40;
			rain1.xpos = LCD_WIDTH + rand % 100;
			rain1.xvelocity -= 1;
			score++;
			WAV_Pause();
			WAV_Play(WAV, 1);
		}
		if (rain2.xpos < rain1.xvelocity) {
			rand = (50021 * rand + 50023) % 50051;
			rain2.ypos = rand % 160 + 40;
			rain2.xpos = LCD_WIDTH + rand % 101;
			rain2.xvelocity -= 1;
			score++;
			WAV_Pause();
			WAV_Play(WAV, 1);
		}

			if (rain1.xvelocity < -150) rain1.xvelocity = -150;
			if (rain2.xvelocity < -150) rain2.xvelocity = -150;

			// User moves the dog
			dog.yvelocity = (BUTTON_DOWN - BUTTON_UP)*7;

			// Bounds check the dog up and down
			if (dog.ypos + dog.yvelocity <= dog.height + 7) {
				dog.ypos = dog.height - dog.yvelocity + 7;
			}
			if (dog.ypos + dog.yvelocity >= LCD_HEIGHT - dog.height - dog.height) {
				dog.ypos = LCD_HEIGHT - dog.height - dog.height - dog.yvelocity;
			}

			
			// Check for collisions with rainbow 1
			x = dog.xpos + dog.width / 2;
			y = dog.ypos + dog.height / 2;

			if (x >= rain1.xpos && 
				x <= rain1.xpos + rain1.width - rain1.xvelocity &&
				((y >= rain1.ypos &&
			      y <= rain1.ypos + rain1.height) ||
			     (dog.ypos+2 >= rain1.ypos &&
			      dog.ypos+2 <= rain1.ypos + rain1.height) ||
			     (dog.ypos+dog.height-2 >= rain1.ypos &&
			      dog.ypos+dog.height-2 <= rain1.ypos+rain1.height))) {

				done = 1;

			}

			// Check for collisions with rainbow 2
			if (x >= rain2.xpos && 
				x <= rain2.xpos + rain2.width - rain2.xvelocity &&
				((y >= rain2.ypos &&
			      y <= rain2.ypos + rain2.height) ||
				 (dog.ypos+2 >= rain2.ypos &&
			      dog.ypos+2 <= rain2.ypos + rain2.height) ||
			     (dog.ypos+dog.height-2 >= rain2.ypos &&
			      dog.ypos+dog.height-2 <= rain2.ypos+rain2.height))) {

			done = 1;

		}

	}


	// End game
	frameUpdateOff();
	WAV_Pause();

	delayms(100);

	LcdFillScreen(LCD_COLOR_BLACK);

	LcdDrawInt(205, 100, score, LCD_COLOR_WHITE, LCD_COLOR_BLACK);

	LcdDrawString(100, 100, "YOU ONLY GOT", LCD_COLOR_WHITE, LCD_COLOR_BLACK);
	LcdDrawString(125, 150, "GOODBYE", LCD_COLOR_WHITE, LCD_COLOR_BLACK);

	return;
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


	initLeds();
	initButtons();
	WAV_Init();
	initLcd();
	initVideo();

	FATFS_LinkDriver(&SD_Driver, SDPath);
    f_mount(&SDFatFs, (TCHAR const*)SDPath, 0);
	

	/*
	 * Initialization is complete. User can press the button to continue at
	 * any time. Until then, a single serial message will be sent, and the
	 * LEDs will continue to light up in a circle.
	 */
	while (!readButton()) {
		i > 8 ? i = 0 : i++;
		ledMap((0xFF >> (8-i)) & 0xFF);
		ledError(e > 2 ? e = 0 : e++);
		delayms(100);
	}
	ledAllOff();
	ledError(0);
	while (readButton());
}

void lcdTest(void) {

	if (initSprite(&rain1, "colorTest.spr")){
		// ERROR
		ledError(LED_ERROR);
	}

	if (initSprite(&dog, "dog.spr")){
		// ERROR
		ledError(LED_ERROR);
	}

	if (initSprite(&rain2, "colorTest.spr")){
		// ERROR
		ledError(LED_ERROR);
	}

	// Test getting a row for video
	rain1.ypos = (LCD_HEIGHT - rain1.height)/2;
	rain1.xpos = (LCD_WIDTH - rain1.width)/2;
	dog.xpos = 220;
	dog.ypos = 170;
	rain2.xpos = 20;
	rain2.ypos = 20;
	spriteLayersAdd(&rain1);
	spriteLayersAdd(&dog);
	spriteLayersAdd(&rain2);
	delayms(1000);

	dog.curFrame = 0;

	// Turn on auto frame updating
	frameUpdateOn();
	
	// Test frame updating with DMA
	while (!readButton()) {
		dog.xvelocity = (BUTTON_RIGHT - BUTTON_LEFT)*5;
		dog.yvelocity = (BUTTON_DOWN - BUTTON_UP)*5;
		rain1.xvelocity = (BUTTON_B - BUTTON_A)*5;
		rain1.yvelocity = (BUTTON_X - BUTTON_Y)*5;
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
	
	// Don't want to free memory the DMA is transferring
	// WAV_Destroy(WAV);

	return;
	
}
