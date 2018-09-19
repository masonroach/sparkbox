#include "lcd.h"

/*
 * +-------------+
 * | Connections |
 * +------+------+
 * | Name | Pin  |
 * +------+------+
 * | CS   | PB7  |
 * | D/C  | PB6  | <-- Stands for data/command
 * | WR   | PB1  |
 * | RD   | PB0  |
 * | RESET| PB8  |
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
// Magic number or something. Gotta try to find this somewhere in the
//   datasheet/reference manual.
volatile uint16_t *fsmc = (uint16_t *)0x60000000;

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
void LcdWriteCmd(LCD_COMMAND cmd) {
	// Pull DC low
	LCD_DC_LOW;

	// Set RD high
	LCD_RD_HIGH;

	// Pull CS low
	LCD_CS_LOW;

	// Pull WR low
	LCD_WR_LOW;

	// Set parallel data
	*fsmc = cmd;
	
	// Pull WR high
	LCD_WR_HIGH;	// Rising edge captures data

	// Set CS high
	LCD_CS_HIGH;
}

// Write data to the LCD controller over FSMC
void LcdWriteData(uint16_t data) {
	// Set DC high
	LCD_DC_HIGH;

	// Pull CS low
	LCD_CS_LOW;

	// Set RD high
	LCD_RD_HIGH;

	// Set parallel data
	*fsmc = data;
	
	// Pull WR low, then high
	LCD_WR_LOW;
	LCD_WR_HIGH;	// Rising edge captures data

	// Set CS high
	LCD_CS_HIGH;

}

void LcdEnterSleep(void) {
	LcdWriteCmd(0x28);	// Display off
	LcdWriteCmd(0x10);	// Enter sleep mode

	return;
}

void LcdExitSleep(void) {
	LcdWriteCmd(0x11);	// Sleep out
	/******************/
	/* DELAY 120MS    */
	/******************/
	LcdWriteCmd(0x29);	// Display On

	return;
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
	// PB[0,1,6,7,8] as outputs
	GPIOB->MODER &= ~(GPIO_MODER_MODER0 |	// Clear mode registers
					  GPIO_MODER_MODER1 |
					  GPIO_MODER_MODER6 |
					  GPIO_MODER_MODER7 |
					  GPIO_MODER_MODER8);
	GPIOB->MODER |=  (GPIO_MODER_MODER0_0 |	// Set modes to output
					  GPIO_MODER_MODER1_0 |
					  GPIO_MODER_MODER6_0 |
					  GPIO_MODER_MODER7_0 |
					  GPIO_MODER_MODER8_0);

	// Rest of the GPIOs as alternate function
	GPIOD->MODER &= ~(GPIO_MODER_MODER0  |	// Clear PDx mode registers
					  GPIO_MODER_MODER1  |
					  GPIO_MODER_MODER8  |
					  GPIO_MODER_MODER9  |
					  GPIO_MODER_MODER10 |
					  GPIO_MODER_MODER14 |
					  GPIO_MODER_MODER15);
	GPIOD->MODER |=   GPIO_MODER_MODER0_1  |	// Set PDx modes to AF
					  GPIO_MODER_MODER1_1  |
					  GPIO_MODER_MODER8_1  |
					  GPIO_MODER_MODER9_1  |
					  GPIO_MODER_MODER10_1 |
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
					   GPIO_AFRL_AFSEL1);
	GPIOD->AFR[1] &= ~(GPIO_AFRH_AFSEL8  |
					   GPIO_AFRH_AFSEL9  |
					   GPIO_AFRH_AFSEL10 |
					   GPIO_AFRH_AFSEL14 |
					   GPIO_AFRH_AFSEL15);
	GPIOD->AFR[0] |= (12 << GPIO_AFRL_AFSEL0_Pos) |	// Set PDx AF to 12 (FSMC)
					 (12 << GPIO_AFRL_AFSEL1_Pos);
	GPIOD->AFR[1] |= (12 << GPIO_AFRH_AFSEL8_Pos)  |
					 (12 << GPIO_AFRH_AFSEL9_Pos)  |
					 (12 << GPIO_AFRH_AFSEL10_Pos) |
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

	GPIOB->OTYPER &= ~(GPIO_OTYPER_OT_0 |	// PBx push-pull
					   GPIO_OTYPER_OT_1 |
					   GPIO_OTYPER_OT_6 |
					   GPIO_OTYPER_OT_7 |
					   GPIO_OTYPER_OT_8);

	GPIOD->OTYPER &= ~(GPIO_OTYPER_OT_0  |	// PDx push-pull
					   GPIO_OTYPER_OT_1  |
					   GPIO_OTYPER_OT_8  |
					   GPIO_OTYPER_OT_9  |
					   GPIO_OTYPER_OT_10 |
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

	GPIOB->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR0 |	// PBx very high speed
					   GPIO_OSPEEDER_OSPEEDR1 |
					   GPIO_OSPEEDER_OSPEEDR6 |
					   GPIO_OSPEEDER_OSPEEDR7 |
					   GPIO_OSPEEDER_OSPEEDR8);

	GPIOD->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR0  |	// PDx very high speed
					   GPIO_OSPEEDER_OSPEEDR1  |
					   GPIO_OSPEEDER_OSPEEDR8  |
					   GPIO_OSPEEDER_OSPEEDR9  |
					   GPIO_OSPEEDER_OSPEEDR10 |
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
	
	GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR0 |	// PBx no pull-up/pull-down
					  GPIO_PUPDR_PUPDR1 |
					  GPIO_PUPDR_PUPDR6 |
					  GPIO_PUPDR_PUPDR7 |
					  GPIO_PUPDR_PUPDR8);
	
	GPIOD->PUPDR &= ~(GPIO_PUPDR_PUPDR0  |	// PDx no pull-up/pull-down
					  GPIO_PUPDR_PUPDR1  |
					  GPIO_PUPDR_PUPDR8  |
					  GPIO_PUPDR_PUPDR9  |
					  GPIO_PUPDR_PUPDR10 |
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
	FSMC_Bank1->BTCR[0] &= ~FSMC_BCR1_ASYNCWAIT;	// Disable sync-wait signal
	FSMC_Bank1->BTCR[0] &= ~FSMC_BCR1_EXTMOD;	// Disable extended mode
	FSMC_Bank1->BTCR[0] &= ~FSMC_BCR1_WAITEN;	// Disable wait bit
	FSMC_Bank1->BTCR[0] |=  FSMC_BCR1_WREN;		// Enable write operations
	FSMC_Bank1->BTCR[0] &= ~FSMC_BCR1_WRAPMOD;	// Disable wrapped burst mode
	FSMC_Bank1->BTCR[0] &= ~FSMC_BCR1_BURSTEN;	// Disable burst mode
	FSMC_Bank1->BTCR[0] |=  FSMC_BCR1_FACCEN;	// Enable flash operations
	FSMC_Bank1->BTCR[0] &= ~FSMC_BCR1_MWID;		// Clear databus width bits
	FSMC_Bank1->BTCR[0] |=  FSMC_BCR1_MWID_0;	// Databus width -> 16-bits
	FSMC_Bank1->BTCR[0] &= ~FSMC_BCR1_MUXEN;	// Disable data multiplexing
	FSMC_Bank1->BTCR[0] |=  FSMC_BCR1_MBKEN;	// Enable memory bank

	FSMC_Bank1->BTCR[1] &= ~FSMC_BTR1_ACCMOD;	// Access mode A
	FSMC_Bank1->BTCR[1] &= ~FSMC_BTR1_BUSTURN;	// Set short bus turn around

	return;
}

/******************************************************************************/
/* FROM THE EXAMPLE CODE PROVIDED, NOT YET UPDATED ****************************/
/******************************************************************************/
static void initILI9341(void) {	 
	volatile uint32_t i;

	LCD_CS_HIGH;
	for (i = 0; i < 20000; i++); //delayms(5);
	LCD_RESET_LOW;
	for (i = 0; i < 50000; i++); //delayms(10);
	LCD_RESET_HIGH;
	for (i = 0; i < 500000; i++); //delayms(120);

	LcdWriteCmd(SOFTWARE_RESET);
	for (i = 0; i < 500000; i++); //delayms(120);
	LcdWriteCmd(DISPLAY_OFF);

/*
 	LcdWriteCmd(0xCF);   
	LcdWriteData(0x00); 
	LcdWriteData(0xc3); 
	LcdWriteData(0X30); 
      
 	LcdWriteCmd(0xED);   
	LcdWriteData(0x64); 
	LcdWriteData(0x03); 
	LcdWriteData(0X12); 
	LcdWriteData(0X81); 
      
 	LcdWriteCmd(0xE8);   
	LcdWriteData(0x85); 
	LcdWriteData(0x10); 
	LcdWriteData(0x79); 
      
 	LcdWriteCmd(0xCB);   
	LcdWriteData(0x39); 
	LcdWriteData(0x2C); 
	LcdWriteData(0x00); 
	LcdWriteData(0x34); 
	LcdWriteData(0x02); 
      
 	LcdWriteCmd(0xF7);   
	LcdWriteData(0x20); 
      
 	LcdWriteCmd(0xEA);   
	LcdWriteData(0x00); 
	LcdWriteData(0x00); 
      
 	LcdWriteCmd(0xC0);    //Power control 
	LcdWriteData(0x22);   //VRH[5:0] 
      
 	LcdWriteCmd(0xC1);    //Power control 
	LcdWriteData(0x11);   //SAP[2:0];BT[3:0] 
      
 	LcdWriteCmd(0xC5);    //VCM control 
	LcdWriteData(0x3d); 
    //LCD_DataWrite_ILI9341(0x30); 
	LcdWriteData(0x20); 
      
 	LcdWriteCmd(0xC7);    //VCM control2 
    //LCD_DataWrite_ILI9341(0xBD); 
	LcdWriteData(0xAA); //0xB0 
    
 	LcdWriteCmd(0x36);    // Memory Access Control 
	LcdWriteData(0x08); 
      
 	LcdWriteCmd(0x3A);   
	LcdWriteData(0x55); 
    
 	LcdWriteCmd(0xB1);   
	LcdWriteData(0x00);   
	LcdWriteData(0x13); 
      
 	LcdWriteCmd(0xB6);    // Display Function Control 
	LcdWriteData(0x0A); 
	LcdWriteData(0xA2); 
    
 	LcdWriteCmd(0xF6);     
	LcdWriteData(0x01); 
	LcdWriteData(0x30); 
      
 	LcdWriteCmd(0xF2);    // 3Gamma Function Disable 
	LcdWriteData(0x00); 
      
 	LcdWriteCmd(0x26);    //Gamma curve selected 
	LcdWriteData(0x01); 
      
 	LcdWriteCmd(0xE0);    //Set Gamma 
	LcdWriteData(0x0F); 
	LcdWriteData(0x3F); 
	LcdWriteData(0x2F); 
	LcdWriteData(0x0C); 
	LcdWriteData(0x10); 
	LcdWriteData(0x0A); 
	LcdWriteData(0x53); 
	LcdWriteData(0XD5); 
	LcdWriteData(0x40); 
	LcdWriteData(0x0A); 
	LcdWriteData(0x13); 
	LcdWriteData(0x03); 
	LcdWriteData(0x08); 
	LcdWriteData(0x03); 
	LcdWriteData(0x00); 
      
 	LcdWriteCmd(0XE1);    //Set Gamma 
	LcdWriteData(0x00); 
	LcdWriteData(0x00); 
	LcdWriteData(0x10); 
	LcdWriteData(0x03); 
	LcdWriteData(0x0F); 
	LcdWriteData(0x05); 
	LcdWriteData(0x2C); 
	LcdWriteData(0xA2); 
	LcdWriteData(0x3F); 
	LcdWriteData(0x05); 
	LcdWriteData(0x0E); 
	LcdWriteData(0x0C); 
	LcdWriteData(0x37); 
	LcdWriteData(0x3C); 
	LcdWriteData(0x0F); 
      
 	LcdWriteCmd(0x11);    //Exit Sleep 
	for (i = 0; i < 500000; i++); //delayms(120);
 	LcdWriteCmd(0x29);    //Display on 
	for (i = 0; i < 200000; i++); //delayms(50);
*/    //Power
    LcdWriteCmd(POWER_1); //power control
    LcdWriteData(0x26);
    LcdWriteCmd(POWER_2); //power control
    LcdWriteData(0x11);
    LcdWriteCmd(VCOM_1); //vcom control
    LcdWriteData(0x5c); //35
    LcdWriteData(0x4c); //3E
    LcdWriteCmd(VCOM_2); //vcom control
    LcdWriteData(0x94);

    //Pixel format
    LcdWriteCmd(PIXEL_FORMAT_SET);
    LcdWriteData(0x65); //16-bit 5-6-5 MCU

    //Memory access
    LcdWriteCmd(MEMORY_ACCESS_CTRL);
    LcdWriteData(0x0048); //Swap row/columns, RGB/BGR swap

    //Frame Rate
    LcdWriteCmd(FRAME_RATE_NORMAL); // frame rate
    LcdWriteData(0x00);
    LcdWriteData(0x1B); //70

    //Gamma
    LcdWriteCmd(ENABLE_3_GAMMA); // 3Gamma Function Disable
    LcdWriteData(0x08);

    LcdWriteCmd(GAMMA_SET);
    LcdWriteData(0x01); // gamma set 4 gamma curve 01/02/04/08

    LcdWriteCmd(POSITIVE_GAMMA_CORRECT); //positive gamma correction
    LcdWriteData(0x1f);
    LcdWriteData(0x1a);
    LcdWriteData(0x18);
    LcdWriteData(0x0a);
    LcdWriteData(0x0f);
    LcdWriteData(0x06);
    LcdWriteData(0x45);
    LcdWriteData(0x87);
    LcdWriteData(0x32);
    LcdWriteData(0x0a);
    LcdWriteData(0x07);
    LcdWriteData(0x02);
    LcdWriteData(0x07);
    LcdWriteData(0x05);
    LcdWriteData(0x00);

    LcdWriteCmd(NEGATIVE_GAMMA_CORRECT); //negative gamma correction
    LcdWriteData(0x00);
    LcdWriteData(0x25);
    LcdWriteData(0x27);
    LcdWriteData(0x05);
    LcdWriteData(0x10);
    LcdWriteData(0x09);
    LcdWriteData(0x3a);
    LcdWriteData(0x78);
    LcdWriteData(0x4d);
    LcdWriteData(0x05);
    LcdWriteData(0x18);
    LcdWriteData(0x0d);
    LcdWriteData(0x38);
    LcdWriteData(0x3a);
    LcdWriteData(0x1f);

    //Set the column and page ranges
    LcdWriteCmd(COLUMN_ADDRESS_SET); // column set
    LcdWriteData(0x00);
    LcdWriteData(0x00);
    LcdWriteData(0x00);
    LcdWriteData(0xEF);
    LcdWriteCmd(PAGE_ADDRESS_SET); // page address set
    LcdWriteData(0x00);
    LcdWriteData(0x00);
    LcdWriteData(0x01);
    LcdWriteData(0x3F);

    //entry mode set
    LcdWriteCmd(ENTRY_MODE_SET);
    LcdWriteData(0x07);

    // display function control
    LcdWriteCmd(DISPLAY_FUNCTION);
    LcdWriteData(0x0a);
    LcdWriteData(0x82);
    LcdWriteData(0x27);
    LcdWriteData(0x00);

    //MDT- for reading data back
    LcdWriteCmd(INTERFACE_CONTROL);
    LcdWriteData(0x01);
    LcdWriteData(0x01);
    LcdWriteData(0x00);

    // Sleep Out
    LcdWriteCmd(SLEEP_OUT);
	for (i = 0; i < 2500000; i++);//    DelayMs(120); //Wait to wake up

    //Display On
    LcdWriteCmd(DISPLAY_ON); //Turn the display on


	return;
}
