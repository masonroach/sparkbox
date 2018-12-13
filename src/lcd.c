/*!
 * @file lcd.c
 * @author Mason Roach
 * @author Patrick Roy
 * @date Dec 12 2018
 *
 * @brief Functions the control the LCD on the Sparkbox
 *
 * These functions are the basic functions that should be used to interface with
 * the ILI9341 LCD controller. Many of these functions were designed for testing
 * and debugging purposes, and are no longer used.
 */

#include "lcd.h"

// Pointer to start address of FSMC
volatile uint16_t * const fsmc_cmd = (uint16_t *)(0x60000000);
volatile uint16_t * const fsmc_data = (uint16_t *)(0x60080000);

/*
 * Prototypes for static functions
 */
static void initFSMC(void);
static void initILI9341(void);

// Configure LCD
void initLcd(void) {
	initFSMC();
	initILI9341();

	return;
}

/*!
 * @brief Write a command to the LCD controller over FSMC
 *
 * @param cmd 16-bits to transfer as a command
 */
void LcdWriteCmd(uint16_t cmd) {

	// Set parallel data
	*fsmc_cmd = cmd;
	
}

/*!
 * @brief Write data to the LCD controller over FSMC
 *
 * @param data 16-bits to transfer as a command parameter or data
 */
void LcdWriteData(uint16_t data) {

	// Set parallel data
	*fsmc_data = data;
	
}

/*!
 * @brief Read data from the LCD controller over FSMC
 */
uint16_t LcdReadData(void) {

	// Read parallel data
	return *fsmc_data;

}

/*!
 * @brief Puts the LCD in sleep mode
 */
void LcdEnterSleep(void) {
	LcdWriteCmd(DISPLAY_OFF);	// Display off
	LcdWriteCmd(ENTER_SLEEP_MODE);	// Enter sleep mode

	return;
}

/*!
 * @brief Removes the LCD from sleep mode
 */
void LcdExitSleep(void) {
	LcdWriteCmd(SLEEP_OUT);	// Sleep out
	delayms(120);
	LcdWriteCmd(DISPLAY_ON);	// Display On

	return;
}

/*!
 * @brief Sets the drawing window for following write commands
 *
 * The ILI9341 controller draws and reads pixels on the LCD through a window
 * method. This function sets the window, but must be followed up with either
 * a MEMORY_WRITE command to draw pixels, or a MEMORY_READ command to read 
 * pixels.
 *
 * @note Be careful of the orientation of axes on the LCD. (0, 0) is in the top
 * left corner
 *
 * @param x0 Start x position
 * @param y0 Start y position
 * @param x1 End x position
 * @param y2 End y position
 */
void LcdSetPos(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
 	LcdWriteCmd(COLUMN_ADDRESS_SET);
	LcdWriteData(x0 >> 8);
	LcdWriteData(x0 & 0xFF);
	LcdWriteData(x1 >> 8);
	LcdWriteData(x1 & 0xFF);
	LcdWriteCmd(PAGE_ADDRESS_SET);
	LcdWriteData(y0 >> 8);
	LcdWriteData(y0 & 0xFF);
	LcdWriteData(y1 >> 8);
	LcdWriteData(y1 & 0xFF);
}

/*!
 * @brief Draws a single pixel at (x, y)
 *
 * @param x x position of the pixel
 * @param y y position of the pixel
 * @param color Color of the pixel to draw, RGB565 format
 */
void LcdPutPixel(uint16_t x, uint16_t y, uint16_t color) {
	LcdSetPos(x, y, x, y);
	LcdWriteCmd(MEMORY_WRITE);
	LcdWriteData(color);
}

/*!
 * @brief Read a single pixel at (x, y)
 *
 * @note Regardless of the data format of pixels written to the LCD, reading a
 * pixel returns data in the RGB888 format.
 *
 * @param x x position of the pixel
 * @param y y position of the pixel
 * 
 * @return The color of the pixel in RGB888 format
 */
uint32_t LcdReadPixel(uint16_t x, uint16_t y) {
	uint32_t temp = 0x00000000;
	LcdSetPos(x, y, x, y);
	LcdWriteCmd(MEMORY_READ);
	LcdReadData();	// Send dummy read signal
	temp = LcdReadData() << 8;	// Get R[7..0], G[7..0]
	temp |= LcdReadData() >> 8;	// Get B[8]
	return temp;
}

/*!
 * @brief Fills the whole LCD screen with a single color
 *
 * @param color RGB565 formatted color
 */
void LcdFillScreen(uint16_t color) {
	uint32_t index = LCD_PIXELS;

	LcdSetPos(0, 0, LCD_WIDTH, LCD_HEIGHT);
	LcdWriteCmd(MEMORY_WRITE);

	LCD_FPS_HIGH;
	
	while (--index) {
		LcdWriteData(color);
	}

	LCD_FPS_LOW;
}

/*!
 * @brief Fills the screen with a checkerboard pattern
 */
void LcdFillScreenCheckered(void) {
	uint32_t index = LCD_PIXELS;
	
	LcdSetPos(0, 0, LCD_WIDTH, LCD_HEIGHT);
	LcdWriteCmd(MEMORY_WRITE);

	while (--index) {
		// Check checkered row, then column, and then xor them
		if ((index%10 >= 5) ^ (index/240 % 10 >= 5)) {
			LcdWriteData(LCD_COLOR_DGRAY);
		} else {
			LcdWriteData(LCD_COLOR_LGRAY);
		}
	}
}

/*!
 * @brief Inverts all colors on the display
 *
 * @param invert Enable (1) or disable (0) the inversion
 */
void LcdInvertDisplay(uint8_t invert) {
	LcdWriteCmd(invert ? INVERSION_ON : INVERSION_OFF);
}

/*!
 * @brief Draws a rectangle of width*height at (x, y)'
 *
 * @param x Starting x position (left side)
 * @param y Starting y position (top side)
 * @param width Width of the rectangle in pixels
 * @param height Height of the rectangle in pixels
 * @param color RGB565 formatted color to fill the rectangle with
 */
void LcdDrawRectangle(uint16_t x, uint16_t y, uint16_t width,
	uint16_t height, uint16_t color) {
	uint32_t index = width*height;
	
	LcdSetPos(x, y, x+width, y+height);
	LcdWriteCmd(MEMORY_WRITE);

	while (--index) {
		LcdWriteData(color);
	}
}

// Decoding array for chars
static const uint64_t charDecode[] = {
	0x0000000000000000,	// 32	Space
	0x0208208208000208,	// 33	!
	0x0,	// 34	"
	0x0492FD24924BF492, // 35	#
	0x0,	// 36	$
	0x0,	// 37	%
	0x0,	// 38	&
	0x0,	// 39	'
	0x0,	// 40	(
	0x0,	// 41	)
	0x0,	// 42	*
	0x0,	// 43	+
	0x0,	// 44	,
	0x0,	// 45	-
	0x0,	// 46	.
	0x0,	// 47	/
	0x07A186186186185E,	// 48	0
	0x021820820820821C,	// 49	1
	0x07A184108421083F,	// 50	2
	0x0FC108438106185E,	// 51	3
	0x00862928BF082087,	// 52	4
	0x0FE0820F8104185E,	// 53	5
	0x01C8420FA186185E,	// 54	6
	0x0FC1042082104104,	// 55	7
	0x07A18617A186185E,	// 56	8
	0x07A186185F042118,	// 57	9
	0x0,	// 58	:
	0x0,	// 59	;
	0x0,	// 60	<
	0x0000000FC003F000,	// 61	=
	0x0,	// 62	>
	0x0,	// 63	?
	0x0,	// 64	@
	0x07A186187F861861,	// 65	A
	0x0FA1861FA186187E,	// 66	B
	0x07A182082082085E,	// 67	C
	0x0FA186186186187E,	// 68	D
	0x0FE0820F2082083F,	// 69	E
	0x0FE0820F20820820,	// 70	F
	0x07A08209E186185E,	// 71	G
	0x0861861FE1861861,	// 72	H
	0x070820820820821C,	// 73	I
	0x078208208249248C,	// 74	J
	0x0862928C30A248A1,	// 75	K
	0x082082082082083E,	// 76	L
	0x08B6DAAAA28A28A2,	// 77	M
	0x08A2CAAAA68A28A2,	// 78	N
	0x07A186186186185E,	// 79	0
	0x0FA1861FA0820820,	// 80	P
	0x07A186186186178F,	// 81	Q
	0x0FA1861FA8922861,	// 82	R
	0x07A182078104185E,	// 83	S
	0x0F88208208208208,	// 84	T
	0x086186186186185E,	// 85	U
	0x086186149249230C,	// 86	V
	0x08A28A28AAAB6DA2,	// 87	W
	0x08A28942085228A2,	// 88	X
	0x08A2894208208208,	// 89	Y
	0x0FC104210842083F	// 90	Z
};

/*!
 * @brief Draw a single character at given (x, y)
 *
 * @param x x position of the top left corner of the character
 * @param y y position of the top left corner of the character
 * @param c Character to draw
 * @param fontColor Color to draw the character as (RGB565 format)
 * @param bgColor Background color of the character (RGB565 format)
 */
void LcdDrawChar(uint16_t x, uint16_t y, uint8_t c,
	uint16_t fontColor, uint16_t bgColor) {

	uint8_t index = 60;
	uint64_t charData = charDecode[c-32];

	LcdSetPos(x, y, x+5, y+9);
	LcdWriteCmd(MEMORY_WRITE);

	while (index--) {
		if ((charData >> index) & 1) LcdWriteData(fontColor);
		else LcdWriteData(bgColor);
	}
}

/*!
 * @brief Draw a string on the LCD
 *
 * @param x x position of the top left corner of the string
 * @param y y position of the top left corner of the string
 * @param c Pointer to null terminated string of characters
 * @param fontColor Color to draw the string as (RGB565 format)
 * @param bgColor Background color of the string (RGB565 format)
 */
void LcdDrawString(uint16_t x, uint16_t y, uint8_t *c,
	uint16_t fontColor, uint16_t bgColor) {

	while (*c != '\0') {
		LcdDrawChar(x, y, *c, fontColor, bgColor);
		x += 7;
		c++;
	}
}

/*!
 * @brief Draw a string on the LCD
 *
 * @param x x position of the top left corner of the string
 * @param y y position of the top left corner of the string
 * @param num Integer to print
 * @param fontColor Color to draw the string as (RGB565 format)
 * @param bgColor Background color of the string (RGB565 format)
 */
void LcdDrawInt(uint16_t x, uint16_t y, uint32_t num,
	uint16_t fontColor, uint16_t bgColor) {
	uint32_t i = 1;

	if (num == 0) {
		LcdDrawChar(x, y, '0', fontColor, bgColor);
	}

	// Find number of digits
	while (num/10 >= i) i*=10;

	// Iterate through the digits from the top down
	while (i) {
		LcdDrawChar(x, y, (num/i % 10)+'0', fontColor, bgColor);
		x += 7;
		i /= 10;
	}
}

/*!
 * @brief Draw a string on the LCD
 *
 * @param x x position of the top left corner of the string
 * @param y y position of the top left corner of the string
 * @param hex Hexadecimal number to print
 * @param fontColor Color to draw the string as (RGB565 format)
 * @param bgColor Background color of the string (RGB565 format)
 */
void LcdDrawHex(uint16_t x, uint16_t y, uint32_t hex,
	uint16_t fontColor, uint16_t bgColor) {
	uint32_t i = 1;

	LcdDrawString(x, y, (uint8_t *)"0X", fontColor, bgColor);
	x += 14;

	if (hex == 0) {
		LcdDrawChar(x, y, '0', fontColor, bgColor);
	}

	// Find number of digits
	while (hex/16 >= i) i*=16;

	// Iterate through the digits from the top down
	while (i) {
		if ((hex/i % 16) < 10) {
			LcdDrawChar(x, y, (hex/i % 16)+'0', fontColor, bgColor);
		} else {
			LcdDrawChar(x, y, (hex/i % 16)-10+'A', fontColor, bgColor);
		}
		x += 7;
		i /= 16;
	}
}

/*!
 * @brief Configure the GPIOs and FSMC port for LCD
 */
static void initFSMC(void) {

	/*
	 * Initialize Clocks
	 */
	RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOAEN |	// Enable GPIOA clock
					 RCC_AHB1ENR_GPIODEN |	// Enable GPIOD clock
					 RCC_AHB1ENR_GPIOEEN);	// Enable GPIOE clock
	RCC->AHB3ENR |=  RCC_AHB3ENR_FSMCEN;	// Enable FSMC clock

	/*
	 * Initialize GPIOs
	 */
	// PA[6,7] as outputs
	GPIOA->MODER &= ~(GPIO_MODER_MODER6 |	// Clear mode registers
					  GPIO_MODER_MODER7);
	GPIOA->MODER |=  (GPIO_MODER_MODER6_0 |	// Set modes to output
					  GPIO_MODER_MODER7_0);

	// Rest of the GPIOs as alternate function
	GPIOD->MODER &= ~(GPIO_MODER_MODER0  |	// Clear PDx mode registers
					  GPIO_MODER_MODER1  |
					  GPIO_MODER_MODER4  |
					  GPIO_MODER_MODER5  |
					  GPIO_MODER_MODER7  |
					  GPIO_MODER_MODER8  |
					  GPIO_MODER_MODER9  |
					  GPIO_MODER_MODER10 |
					  GPIO_MODER_MODER13 |
					  GPIO_MODER_MODER14 |
					  GPIO_MODER_MODER15);
	GPIOD->MODER |=   GPIO_MODER_MODER0_1  |	// Set PDx modes to AF
					  GPIO_MODER_MODER1_1  |
					  GPIO_MODER_MODER4_1  |
					  GPIO_MODER_MODER5_1  |
					  GPIO_MODER_MODER7_1  |
					  GPIO_MODER_MODER8_1  |
					  GPIO_MODER_MODER9_1  |
					  GPIO_MODER_MODER10_1 |
					  GPIO_MODER_MODER13_1 |
					  GPIO_MODER_MODER14_1 |
					  GPIO_MODER_MODER15_1;


	GPIOE->MODER &= ~(GPIO_MODER_MODER7  |	// Clear PEx mode registers
					  GPIO_MODER_MODER8  |
					  GPIO_MODER_MODER9  |
					  GPIO_MODER_MODER10 |
					  GPIO_MODER_MODER11 |
					  GPIO_MODER_MODER12 |
					  GPIO_MODER_MODER13 |
					  GPIO_MODER_MODER14 |
					  GPIO_MODER_MODER15);
	GPIOE->MODER |=   GPIO_MODER_MODER7_1  |	// Set PEx modes to AF
					  GPIO_MODER_MODER8_1  |
					  GPIO_MODER_MODER9_1  |
					  GPIO_MODER_MODER10_1 |
					  GPIO_MODER_MODER11_1 |
					  GPIO_MODER_MODER12_1 |
					  GPIO_MODER_MODER13_1 |
					  GPIO_MODER_MODER14_1 |
					  GPIO_MODER_MODER15_1;

	GPIOD->AFR[0] &= ~(GPIO_AFRL_AFSEL0 |	// Clear PDx AF registers
					   GPIO_AFRL_AFSEL1 |
					   GPIO_AFRL_AFSEL4 |
					   GPIO_AFRL_AFSEL5 |
					   GPIO_AFRL_AFSEL7);
	GPIOD->AFR[1] &= ~(GPIO_AFRH_AFSEL8  |
					   GPIO_AFRH_AFSEL9  |
					   GPIO_AFRH_AFSEL10 |
					   GPIO_AFRH_AFSEL13 |
					   GPIO_AFRH_AFSEL14 |
					   GPIO_AFRH_AFSEL15);
	GPIOD->AFR[0] |= (12 << GPIO_AFRL_AFSEL0_Pos) |	// Set PDx AF to 12 (FSMC)
					 (12 << GPIO_AFRL_AFSEL1_Pos) |
					 (12 << GPIO_AFRL_AFSEL4_Pos) |
					 (12 << GPIO_AFRL_AFSEL5_Pos) |
					 (12 << GPIO_AFRL_AFSEL7_Pos);
	GPIOD->AFR[1] |= (12 << GPIO_AFRH_AFSEL8_Pos)  |
					 (12 << GPIO_AFRH_AFSEL9_Pos)  |
					 (12 << GPIO_AFRH_AFSEL10_Pos) |
					 (12 << GPIO_AFRH_AFSEL13_Pos) |
					 (12 << GPIO_AFRH_AFSEL14_Pos) |
					 (12 << GPIO_AFRH_AFSEL15_Pos);
	

	GPIOE->AFR[0] &= ~(GPIO_AFRL_AFSEL7);	// Clear PEx AF registers
	GPIOE->AFR[1] &= ~(GPIO_AFRH_AFSEL8  |
					   GPIO_AFRH_AFSEL9  |
					   GPIO_AFRH_AFSEL10 |
					   GPIO_AFRH_AFSEL11 |
					   GPIO_AFRH_AFSEL12 |
					   GPIO_AFRH_AFSEL13 |
					   GPIO_AFRH_AFSEL14 |
					   GPIO_AFRH_AFSEL15);
	GPIOE->AFR[0] |= (12 << GPIO_AFRL_AFSEL7_Pos);	//Set PEx AF to 12 (FSMC)
	GPIOE->AFR[1] |= (12 << GPIO_AFRH_AFSEL8_Pos)  |
					 (12 << GPIO_AFRH_AFSEL9_Pos)  |
					 (12 << GPIO_AFRH_AFSEL10_Pos) |
					 (12 << GPIO_AFRH_AFSEL11_Pos) |
					 (12 << GPIO_AFRH_AFSEL12_Pos) |
					 (12 << GPIO_AFRH_AFSEL13_Pos) |
					 (12 << GPIO_AFRH_AFSEL14_Pos) |
					 (12 << GPIO_AFRH_AFSEL15_Pos);

	GPIOA->OTYPER &= ~(GPIO_OTYPER_OT_6 |	// PBx push-pull
					   GPIO_OTYPER_OT_7);

	GPIOD->OTYPER &= ~(GPIO_OTYPER_OT_0  |	// PDx push-pull
					   GPIO_OTYPER_OT_1  |
					   GPIO_OTYPER_OT_4  |
					   GPIO_OTYPER_OT_5  |
					   GPIO_OTYPER_OT_7  |
					   GPIO_OTYPER_OT_8  |
					   GPIO_OTYPER_OT_9  |
					   GPIO_OTYPER_OT_10 |
					   GPIO_OTYPER_OT_13 |
					   GPIO_OTYPER_OT_14 |
					   GPIO_OTYPER_OT_15);

	GPIOE->OTYPER &= ~(GPIO_OTYPER_OT_7  |	// PEx push-pull
					   GPIO_OTYPER_OT_8  |
					   GPIO_OTYPER_OT_9  |
					   GPIO_OTYPER_OT_10 |
					   GPIO_OTYPER_OT_11 |
					   GPIO_OTYPER_OT_12 |
					   GPIO_OTYPER_OT_13 |
					   GPIO_OTYPER_OT_14 |
					   GPIO_OTYPER_OT_15);

	GPIOA->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR6 |	// PBx very high speed
					   GPIO_OSPEEDER_OSPEEDR7);

	GPIOD->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR0  |	// PDx very high speed
					   GPIO_OSPEEDER_OSPEEDR1  |
					   GPIO_OSPEEDER_OSPEEDR4  |
					   GPIO_OSPEEDER_OSPEEDR5  |
					   GPIO_OSPEEDER_OSPEEDR7  |
					   GPIO_OSPEEDER_OSPEEDR8  |
					   GPIO_OSPEEDER_OSPEEDR9  |
					   GPIO_OSPEEDER_OSPEEDR10 |
					   GPIO_OSPEEDER_OSPEEDR13 |
					   GPIO_OSPEEDER_OSPEEDR14 |
					   GPIO_OSPEEDER_OSPEEDR15);

	GPIOE->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR7  |	// PEx very high speed
					   GPIO_OSPEEDER_OSPEEDR8  |
					   GPIO_OSPEEDER_OSPEEDR9  |
					   GPIO_OSPEEDER_OSPEEDR10 |
					   GPIO_OSPEEDER_OSPEEDR11 |
					   GPIO_OSPEEDER_OSPEEDR12 |
					   GPIO_OSPEEDER_OSPEEDR13 |
					   GPIO_OSPEEDER_OSPEEDR14 |
					   GPIO_OSPEEDER_OSPEEDR15);
	
	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR6 |	// PBx clear pull-up/pull-down
					  GPIO_PUPDR_PUPDR7);
	
	GPIOD->PUPDR &= ~(GPIO_PUPDR_PUPDR0  |	// PDx clear pull-up/pull-down
					  GPIO_PUPDR_PUPDR1  |
					  GPIO_PUPDR_PUPDR4  |
					  GPIO_PUPDR_PUPDR5  |
					  GPIO_PUPDR_PUPDR7  |
					  GPIO_PUPDR_PUPDR8  |
					  GPIO_PUPDR_PUPDR9  |
					  GPIO_PUPDR_PUPDR10 |
					  GPIO_PUPDR_PUPDR13 |
					  GPIO_PUPDR_PUPDR14 |
					  GPIO_PUPDR_PUPDR15);

	GPIOE->PUPDR &= ~(GPIO_PUPDR_PUPDR7  |	// PEx clear pull-up/pull-down
					  GPIO_PUPDR_PUPDR8  |
					  GPIO_PUPDR_PUPDR9  |
					  GPIO_PUPDR_PUPDR10 |
					  GPIO_PUPDR_PUPDR11 |
					  GPIO_PUPDR_PUPDR12 |
					  GPIO_PUPDR_PUPDR13 |
					  GPIO_PUPDR_PUPDR14 |
					  GPIO_PUPDR_PUPDR15);

	/*
	 * Initialize FSMC using bank 1 NORSRAM1
	 */
	FSMC_Bank1->BTCR[0] &= ~FSMC_BCR1_CBURSTRW;	// Burst mode disable
	FSMC_Bank1->BTCR[0] &= ~FSMC_BCR1_ASYNCWAIT;	// Disable sync-wait signal
	FSMC_Bank1->BTCR[0] &= ~FSMC_BCR1_EXTMOD;	// Disable extended mode
	FSMC_Bank1->BTCR[0] &= ~FSMC_BCR1_WAITEN;	// Disable wait bit
	FSMC_Bank1->BTCR[0] |=  FSMC_BCR1_WREN;		// Enable write operations
	FSMC_Bank1->BTCR[0] &= ~FSMC_BCR1_WAITCFG;	// Wait timing before WRITE
	FSMC_Bank1->BTCR[0] &= ~FSMC_BCR1_WRAPMOD;	// Disable wrapped burst mode
	FSMC_Bank1->BTCR[0] &= ~FSMC_BCR1_BURSTEN;	// Disable burst mode
	FSMC_Bank1->BTCR[0] |=  FSMC_BCR1_FACCEN;	// Enable flash operations
	FSMC_Bank1->BTCR[0] &= ~FSMC_BCR1_WAITPOL;	// Wait signal polarity low
	FSMC_Bank1->BTCR[0] &= ~FSMC_BCR1_MWID;		// Clear databus width bits
	FSMC_Bank1->BTCR[0] |=  FSMC_BCR1_MWID_0;	// Databus width -> 16-bits
	FSMC_Bank1->BTCR[0] &= ~FSMC_BCR1_MTYP;		// Memory type -> SRAM
	FSMC_Bank1->BTCR[0] &= ~FSMC_BCR1_MUXEN;	// Disable data multiplexing
	FSMC_Bank1->BTCR[0] |=  FSMC_BCR1_MBKEN;	// Enable memory bank

	FSMC_Bank1->BTCR[1] &= ~FSMC_BTR1_ACCMOD;	// Access mode A
	FSMC_Bank1->BTCR[1] &= ~FSMC_BTR1_DATAST;	// Clear DATAST bits
	FSMC_Bank1->BTCR[1] |= (4 << FSMC_BTR1_DATAST_Pos);	// DATAST = 4 x HCLK
	FSMC_Bank1->BTCR[1] &= ~FSMC_BTR1_BUSTURN;	// Clear bus turn around bits
	FSMC_Bank1->BTCR[1] |=  FSMC_BTR1_ADDHLD;	// Enable Address hold
	FSMC_Bank1->BTCR[1] &= ~FSMC_BTR1_ADDSET;	// Clear Address hold
	FSMC_Bank1->BTCR[1] |= (4 << FSMC_BTR1_ADDSET_Pos); // Set Address hold

	return;
}

/*!
 * @brief Initialize the ILI9341 LCD controller
 */
static void initILI9341(void) {
	// Hardware reset
	LCD_RESET_HIGH;
	delayms(5);
	LCD_RESET_LOW;
	delayms(10);
	LCD_RESET_HIGH;
	delayms(250);
	
	LcdWriteCmd(POWER_A);  
	LcdWriteData(0x39); 
	LcdWriteData(0x2C); 
	LcdWriteData(0x00); 
	LcdWriteData(0x34); 
	LcdWriteData(0x02); 

	LcdWriteCmd(POWER_B);  
	LcdWriteData(0x00); 
	LcdWriteData(0XC1); 
	LcdWriteData(0X30); 

	LcdWriteCmd(DRIVER_TIMING_CONTROL_A);  
	LcdWriteData(0x85); 
	LcdWriteData(0x00); 
	LcdWriteData(0x78); 

	LcdWriteCmd(DRIVER_TIMING_CONTROL_B);
	LcdWriteData(0x00); 
	LcdWriteData(0x00); 
 
	LcdWriteCmd(POWER_ON_SEQUENCE_CTRL);  
	LcdWriteData(0x64); 
	LcdWriteData(0x03); 
	LcdWriteData(0X12); 
	LcdWriteData(0X81); 

	LcdWriteCmd(PUMP_RATIO_CONTROL);
	LcdWriteData(0x20); 
  
	LcdWriteCmd(POWER_1);	//Power control 
	LcdWriteData(0x19);   //VRH[5:0] 
 
	LcdWriteCmd(POWER_2);	//Power control 
	LcdWriteData(0x11);   //SAP[2:0];BT[3:0] 

	LcdWriteCmd(VCOM_1);	//VCM control 
	LcdWriteData(0x3d);   //Contrast
	LcdWriteData(0x2B); 
 
	LcdWriteCmd(VCOM_2);	//VCM control2 
	LcdWriteData(0xC1);   //--
 
	LcdWriteCmd(MEMORY_ACCESS_CTRL);	// Memory Access Control 
	LcdWriteData(0x3C);	// Horizontal RGB

	LcdWriteCmd(PIXEL_FORMAT_SET);	
	LcdWriteData(0x55); 

	LcdWriteCmd(FRAME_RATE_NORMAL);
	LcdWriteData(0x00);
	LcdWriteData(0x18);
 
	LcdWriteCmd(DISPLAY_FUNCTION);	// Display Function Control 
	LcdWriteData(0x08); 
	LcdWriteData(0x82);
	LcdWriteData(0x27);  
 
	LcdExitSleep();

	return;
}
