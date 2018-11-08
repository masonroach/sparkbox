#include "lcd.h"
/*
 * +-------------+
 * | Connections |
 * +------+------+
 * | Name | Pin  |
 * +------+------+
 * | FRAME| PB9  | <-- Trigger on new frame
 * | RESET| PB8  |
 * | NOE  | PD4  | <-- Read
 * | NWE  | PD5  | <-- Write
 * | NE1  | PD7  | <-- Chip Select
 * | A18  | PD13 | <-- Register Select (C/D)
 * | DB0  | PD14 |
 * | DB1  | PD15 |
 * | DB2  | PD0  |
 * | DB3  | PD1  |
 * | DB4  | PE7  |
 * | DB5  | PE8  |
 * | DB6  | PE9  |
 * | DB7  | PE10 |
 * | DB8  | PE11 |
 * | DB9  | PE12 |
 * | DB10 | PE13 |
 * | DB11 | PE14 |
 * | DB12 | PE15 |
 * | DB13 | PD8  |
 * | DB14 | PD9  |
 * | DB15 | PD10 |
 * +------+------+
 */

// Pointer to start address of FSMC
static volatile uint16_t *fsmc_cmd = (uint16_t *)(0x60000000);
static volatile uint16_t *fsmc_data = (uint16_t *)(0x60080000);

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

// Write a command to the LCD controller over FSMC
void LcdWriteCmd(uint16_t cmd) {

	// Set parallel data
	*fsmc_cmd = cmd;
	
}

// Write data to the LCD controller over FSMC
void LcdWriteData(uint16_t data) {

	// Set parallel data
	*fsmc_data = data;
	
}

// Read data from the LCD controller over FSMC
uint16_t LcdReadData(void) {

	// Read parallel data
	return *fsmc_data;

}

void LcdEnterSleep(void) {
	LcdWriteCmd(DISPLAY_OFF);	// Display off
	LcdWriteCmd(ENTER_SLEEP_MODE);	// Enter sleep mode

	return;
}

void LcdExitSleep(void) {
	LcdWriteCmd(SLEEP_OUT);	// Sleep out
	delayms(120);
	LcdWriteCmd(DISPLAY_ON);	// Display On

	return;
}

// Sets the drawing window for following write commands
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

// Draws a single pixel at (x, y)
void LcdPutPixel(uint16_t x, uint16_t y, uint16_t color) {
	LcdSetPos(x, y, x, y);
	LcdWriteCmd(MEMORY_WRITE);
	LcdWriteData(color);
}

// Read a single pixel at (x, y)
uint16_t LcdReadPixel(uint16_t x, uint16_t y) {
	uint16_t temp = 0x0000;
	LcdSetPos(x, y, x, y);
	LcdWriteCmd(MEMORY_READ);
	LcdReadData();	// Send dummy read signal
	temp = LcdReadData();	// Get R[7..0], G[7..0]
	temp = (temp & 0xF800) | ((temp & 0xFC) << 3);	// Convert to R[5], G[6]
	temp |= LcdReadData() >> 11;	// Get B[8], convetrt to B[5]
	return temp;
}

// Fills the whole LCD screen with a single color
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

// Fills the screen with a checkerboard pattern
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

// Draws a rectangle of width*height at (x, y)
void LcdDrawRectangle(uint16_t x, uint16_t y, uint16_t width,
	uint16_t height, uint16_t color) {
	uint32_t index = width*height;
	
	LcdSetPos(x, y, x+width, y+height);
	LcdWriteCmd(MEMORY_WRITE);

	while (--index) {
		LcdWriteData(color);
	}
}

// Inverts all colors on the display
void LcdInvertDisplay(uint8_t invert) {
	LcdWriteCmd(invert ? INVERSION_ON : INVERSION_OFF);
}

// Optional text functions
#if LCD_TEXT==1
// Decoding array for chars
const uint64_t charDecode[] = {
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

// Draw a single character at given (x, y)
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

void LcdDrawString(uint16_t x, uint16_t y, uint8_t *c,
	uint16_t fontColor, uint16_t bgColor) {

	while (*c != '\0') {
		LcdDrawChar(x, y, *c, fontColor, bgColor);
		x += 7;
		c++;
	}
}

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

void LcdDrawHex(uint16_t x, uint16_t y, uint32_t hex,
	uint16_t fontColor, uint16_t bgColor) {
	uint32_t i = 1;

	LcdDrawString(x, y, (uint8_t *)"0X", fontColor, bgColor);
	x += 14;

	if (hex == 0) {
		LcdDrawChar(x, y, '0', fontColor, bgColor);
	}

	// Find number of digits
	while (hex/16 > i) i*=16;

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
#endif

// Configure the FSMC port for LCD
static void initFSMC(void) {

	/*
	 * Initialize Clocks
	 */
	RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOBEN |	// Enable GPIOB clock
					 RCC_AHB1ENR_GPIODEN |	// Enable GPIOD clock
					 RCC_AHB1ENR_GPIOEEN);	// Enable GPIOE clock
	RCC->AHB3ENR |=  RCC_AHB3ENR_FSMCEN;	// Enable FSMC clock

	/*
	 * Initialize GPIOs
	 */
	// PB[8,9] as outputs
	GPIOB->MODER &= ~(GPIO_MODER_MODER8 |	// Clear mode registers
					  GPIO_MODER_MODER9);
	GPIOB->MODER |=  (GPIO_MODER_MODER8_0 |	// Set modes to output
					  GPIO_MODER_MODER9_0);

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

	GPIOB->OTYPER &= ~(GPIO_OTYPER_OT_8 |	// PBx push-pull
					   GPIO_OTYPER_OT_9);

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

	GPIOB->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR8 |	// PBx very high speed
					   GPIO_OSPEEDER_OSPEEDR9);

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
	
	GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR8 |	// PBx clear pull-up/pull-down
					  GPIO_PUPDR_PUPDR9);
	
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
	FSMC_Bank1->BTCR[1] |= (12 << FSMC_BTR1_DATAST_Pos);	// DATAST = 4 x HCLK
	FSMC_Bank1->BTCR[1] &= ~FSMC_BTR1_BUSTURN;	// Clear bus turn around bits
	FSMC_Bank1->BTCR[1] |=  FSMC_BTR1_ADDHLD;	// Enable Address hold
	FSMC_Bank1->BTCR[1] &= ~FSMC_BTR1_ADDSET;	// Clear Address hold
	FSMC_Bank1->BTCR[1] |= (12 << FSMC_BTR1_ADDSET_Pos); // Set Address hold

	return;
}

/******************************************************************************/
/* FROM PROVIDED EXAMPLE CODE, DO NOT CHANGE YET   ****************************/
/******************************************************************************/
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
	LcdWriteData(0xE8);	// What we need
//	LcdWriteData(0x08);	// Default/fast

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
