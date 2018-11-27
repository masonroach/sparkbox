#ifndef SPARK_LCD
#define SPARK_LCD

#include "stm32f4xx.h"
#include "clock.h"

/*
 * +-------------+
 * | Connections |
 * +------+------+
 * | Name | Pin  |
 * +------+------+
 * | FRAME| PA7  | <-- Trigger on new frame
 * | RESET| PA6  |
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

//#define LCD_RESET_HIGH	GPIOB->ODR |= (1 << 8)
//#define LCD_RESET_LOW	GPIOB->ODR &= ~(1 << 8)
#define LCD_RESET_HIGH	GPIOA->ODR |= (1 << 6)
#define LCD_RESET_LOW	GPIOA->ODR &= ~(1 << 6)
//#define LCD_FPS_HIGH	GPIOB->ODR |= (1 << 9)
//#define LCD_FPS_LOW		GPIOB->ODR &= ~(1 << 9)
#define LCD_FPS_HIGH	GPIOA->ODR |= (1 << 7)
#define LCD_FPS_LOW		GPIOA->ODR &= ~(1 << 7)

// LCD dimensions
#define LCD_WIDTH 320
#define LCD_HEIGHT 240
#define LCD_PIXELS LCD_WIDTH*LCD_HEIGHT

// LCD sample colors (565 Format)
#define LCD_COLOR_BLUE	 0x001F
#define LCD_COLOR_RED	 0xF800
#define LCD_COLOR_GREEN	 0x07E0
#define LCD_COLOR_BLACK  0x0000
#define LCD_COLOR_WHITE  0xFFFF
#define LCD_COLOR_LGRAY  0xE73C
#define LCD_COLOR_DGRAY  0x18E3
#define LCD_COLOR_YELLOW 0xFFE0

typedef enum {
	NOP = 0x00,
	SOFTWARE_RESET = 0x01,
	READ_ID = 0x04,
	READ_STATUS = 0x09,
	READ_POWER_MODE = 0x0A,
	READ_MADCTL = 0x0B,
	READ_PIXEL_FORMAT = 0x0C,
	READ_IMAGE_FORMAT = 0x0D,
	READ_SIGNAL_MODE = 0x0E,
	READ_DIAGNOSTIC = 0x0F,
	ENTER_SLEEP_MODE = 0x10,
	SLEEP_OUT = 0x11,
	PARTIAL_MODE_ON = 0x12,
	NORMAL_MODE_ON = 0x13,
	INVERSION_OFF = 0x20,
	INVERSION_ON = 0x21,
	GAMMA_SET = 0x26,
	DISPLAY_OFF = 0x28,
	DISPLAY_ON = 0x29,
	COLUMN_ADDRESS_SET = 0x2A,
	PAGE_ADDRESS_SET = 0x2B,
	MEMORY_WRITE = 0x2C,
	COLOR_SET = 0x2D,
	MEMORY_READ = 0x2E,
	PARTIAL_AREA = 0x30,
	VERTSCROLL_DEF = 0x33,
	TEARING_LINE_OFF = 0x34,
	TEARING_LINE_ON = 0x35,
	MEMORY_ACCESS_CTRL = 0x36,
	VERTSCROLL_START_ADDR = 0x37,
	IDLE_MODE_OFF = 0x38,
	IDLE_MODE_ON = 0x39,
	PIXEL_FORMAT_SET = 0x3A,
	WRITE_MEMORY_CONTINUE = 0x3C,
	READ_MEMORY_CONTINUE = 0x3E,
	SET_TEAR_SCANLINE = 0x44,
	GET_SCANLINE = 0x45,
	WRITE_DISPLAY_BRIGHT = 0x51,
	READ_DISPLAY_BRIGHT = 0x52,
	WRITE_CTRL_DISPLAY = 0x53,
	READ_CTRL_DISPLAY = 0x54,
	WRITE_CONTADAPT_BRIGHT = 0x55,
	READ_CONTADAPT_BRIGHT = 0x56,
	WRITE_CABC_MIN_BRIGHT = 0x5E,
	READ_CABC_MIN_BRIGHT = 0x5F,
	READ_ID1 = 0xDA,
	READ_ID2 = 0xDB,
	READ_ID3 = 0xDC,
	RGB_INTERFACE_SIGNAL = 0xB0,
	FRAME_RATE_NORMAL = 0xB1,
	FRAME_RATE_IDLE = 0xB2,
	FRAME_RATE_PARTIAL = 0xB3,
	DISPLAY_INVERSION = 0xB4,
	BLANKING_PORCH = 0xB5,
	DISPLAY_FUNCTION = 0xB6,
	ENTRY_MODE_SET = 0xB7,
	BACKLIGHT_1 = 0xB8,
	BACKLIGHT_2 = 0xB9,
	BACKLIGHT_3 = 0xBA,
	BACKLIGHT_4 = 0xBB,
	BACKLIGHT_5 = 0xBC,
	BACKLIGHT_6 = 0xBD,
	BACKLIGHT_7 = 0xBE,
	BACKLIGHT_8 = 0xBF,
	POWER_1 = 0xC0,
	POWER_2 = 0xC1,
	VCOM_1 = 0xC5,
	VCOM_2 = 0xC7,
	POWER_A = 0xCB,
	POWER_B = 0xCF,
	NV_MEMORY_WRITE = 0xD0,
	NV_MEMORY_PROTECT_KEY = 0xD1,
	NV_MEMORY_STATUS_READ = 0xD2,
	READ_ID4 = 0xD3,
	POSITIVE_GAMMA_CORRECT = 0xE0,
	NEGATIVE_GAMMA_CORRECT = 0xE1,
	DIGITAL_GAMMA_CONTROL_1	= 0xE2,
	DIGITAL_GAMMA_CONTROL_2	= 0xE3,
	INTERFACE_CONTROL = 0xF6,
	POWER_CONTROL_A = 0xCB,
	POWER_CONTROL_B = 0xCF,
	DRIVER_TIMING_CONTROL_A	= 0xE8,
	DRIVER_TIMING_CONTROL_B = 0xEA,
	POWER_ON_SEQUENCE_CTRL = 0xED,
	ENABLE_3_GAMMA = 0xF2,
	PUMP_RATIO_CONTROL = 0xF7
} LCD_COMMAND;

void initLcd(void);
void LcdWriteCmd(uint16_t cmd);
void LcdWriteData(uint16_t data);
uint16_t LcdReadData(void);
void LcdEnterSleep(void);
void LcdExitSleep(void);
void LcdPutPixel(uint16_t x, uint16_t y, uint16_t color);
uint16_t LcdReadPixel(uint16_t x, uint16_t y);
void LcdFillScreen(uint16_t color);
void LcdFillScreenCheckered(void);
void LcdInvertDisplay(uint8_t invert);
void LcdSetPos(uint16_t x0, uint16_t x1, uint16_t y0, uint16_t y1);
void LcdDrawRectangle(uint16_t x, uint16_t y, uint16_t width,
	uint16_t height, uint16_t color);

// Change this to include text decoding functions
#define LCD_TEXT 1
#if LCD_TEXT==1
void LcdDrawChar(uint16_t x, uint16_t y, uint8_t c,
	uint16_t fontColor, uint16_t bgColor);
void LcdDrawString(uint16_t x, uint16_t y, uint8_t *c,
	uint16_t fontColor, uint16_t bgColor);
void LcdDrawInt(uint16_t x, uint16_t y, uint32_t num,
	uint16_t fontColor, uint16_t bgColor);
void LcdDrawHex(uint16_t x, uint16_t y, uint32_t hex,
	uint16_t fontColor, uint16_t bgColor);
#endif

#endif
