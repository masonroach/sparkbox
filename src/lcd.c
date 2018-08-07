#include "lcd.h"

/*
 * +-------------+
 * | Connections |
 * +------+------+
 * | Name | Pin  |
 * +------+------+
 * | CS   | PB7  |
 * | DC   | PB6  | <-- Stands for data/command
 * | WR   | PB1  |
 * | RESET| PB8  |
 * | RD   | PB0  |
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

// Configure LCD
void initLCD(void) {
	initFSMC();

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
