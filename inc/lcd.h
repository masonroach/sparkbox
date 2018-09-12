#include "stm32f4xx.h"

#ifndef SPARK_LCD
#define SPARK_LCD

/*
 * +-------------+
 * | Connections |
 * +------+------+
 * | Name | Pin  |
 * +------+------+
 * | CS   | PB7  |
 * | D/C  | PB6  | <-- Stands for data/command
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

#define LCD_CS_HIGH()	GPIOB->ODR |= (1 << 7)
#define LCD_CS_LOW()	GPIOB->ODR &= ~(1 << 7)
#define LCD_DC_HIGH()	GPIOB->ODR |= (1 << 6)
#define LCD_DC_LOW()	GPIOB->ODR &= ~(1 << 6)
#define LCD_WR_HIGH()	GPIOB->ODR |= (1 << 1)
#define LCD_WR_LOW()	GPIOB->ODR &= ~(1 << 1)
#define LCD_RESET_HIGH()	GPIOB->ODR |= (1 << 8)
#define LCD_RESET_LOW()		GPIOB->ODR &= ~(1 << 8)
#define LCD_RD_HIGH()	GPIOB->ODR |= 1
#define LCD_RD_LOW()	GPIOB->ODR &= ~1

void initLcd(void);
void LcdWriteCmd(uint16_t command);
static void initFSMC(void);

#endif
