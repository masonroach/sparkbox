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
static void LcdSetPos(uint16_t x0, uint16_t x1, uint16_t y0, uint16_t y1);

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

void LcdPutPixel(uint16_t x, uint16_t y, uint16_t color) {
	LcdSetPos(x, y, x, y);
	LcdWriteData(color);
}

void LcdFillScreen(uint16_t color) {
	uint32_t index = LCD_PIXELS;
	
	LcdSetPos(0, 0, LCD_WIDTH, LCD_HEIGHT);
	while (index--) {
		LcdWriteData(color);
	}
}

void LcdInvertDisplay(uint8_t invert) {
	LcdWriteCmd(invert ? INVERSION_ON : INVERSION_OFF);
}

static void LcdSetPos(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
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
	LcdWriteCmd(MEMORY_WRITE);
}

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
	
	GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR8 |	// PBx no pull-up/pull-down
					  GPIO_PUPDR_PUPDR9);
	
	GPIOD->PUPDR &= ~(GPIO_PUPDR_PUPDR0  |	// PDx no pull-up/pull-down
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

	GPIOE->PUPDR &= ~(GPIO_PUPDR_PUPDR7  |	// PEx no pull-up/pull-down
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
	FSMC_Bank1->BTCR[1] |= (8 << FSMC_BTR1_DATAST_Pos);	// DATAST = 4 x HCLK
	FSMC_Bank1->BTCR[1] &= ~FSMC_BTR1_BUSTURN;	// Clear bus turn around bits
	FSMC_Bank1->BTCR[1] |=  FSMC_BTR1_ADDHLD;	// Enable Address hold
	FSMC_Bank1->BTCR[1] &= ~FSMC_BTR1_ADDSET;	// Clear Address hold
	FSMC_Bank1->BTCR[1] |= (8 << FSMC_BTR1_ADDSET_Pos); // Set Address hold

	return;
}

/******************************************************************************/
/* FROM PROVIDED EXAMPLE CODE, DO NOT CHANGE YET   ****************************/
/******************************************************************************/
static void initILI9341(void) {	 
	LCD_RESET_HIGH;
	delayms(5);
	LCD_RESET_LOW;
	delayms(10);
	LCD_RESET_HIGH;
	delayms(250);

	LcdWriteCmd(0xCB);  
    LcdWriteData(0x39); 
    LcdWriteData(0x2C); 
    LcdWriteData(0x00); 
    LcdWriteData(0x34); 
    LcdWriteData(0x02); 

    LcdWriteCmd(0xCF);  
    LcdWriteData(0x00); 
    LcdWriteData(0XC1); 
    LcdWriteData(0X30); 

    LcdWriteCmd(0xE8);  
    LcdWriteData(0x85); 
    LcdWriteData(0x00); 
    LcdWriteData(0x78); 

    LcdWriteCmd(0xEA);  
    LcdWriteData(0x00); 
    LcdWriteData(0x00); 
 
    LcdWriteCmd(0xED);  
    LcdWriteData(0x64); 
    LcdWriteData(0x03); 
    LcdWriteData(0X12); 
    LcdWriteData(0X81); 

    LcdWriteCmd(0xF7);  
    LcdWriteData(0x20); 
  
    LcdWriteCmd(0xC0);    //Power control 
    LcdWriteData(0x19);   //VRH[5:0] 
 
    LcdWriteCmd(0xC1);    //Power control 
    LcdWriteData(0x11);   //SAP[2:0];BT[3:0] 

    LcdWriteCmd(0xC5);    //VCM control 
    LcdWriteData(0x3d);   //Contrast
    LcdWriteData(0x2B); 
 
    LcdWriteCmd(0xC7);    //VCM control2 
    LcdWriteData(0xC1);   //--
 
    LcdWriteCmd(0x36);    // Memory Access Control 
    LcdWriteData(0x48);   

    LcdWriteCmd(0x3A);    
    LcdWriteData(0x55); 

    LcdWriteCmd(0xB1);    
    LcdWriteData(0x00);  
    LcdWriteData(0x18); 
 
    LcdWriteCmd(0xB6);    // Display Function Control 
    LcdWriteData(0x08); 
    LcdWriteData(0x82);
    LcdWriteData(0x27);  
 
    LcdWriteCmd(0x11);    //Exit Sleep 
    delayms(120); 
				
    LcdWriteCmd(0x29);    //Display on 
    LcdWriteCmd(0x2c);

	return;
}
