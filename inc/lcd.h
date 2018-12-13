/*!
 * @file lcd.h
 * @author Mason Roach
 * @author Patrick Roy
 * @date Dec 12 2018
 *
 * @brief Functions the control the LCD on the Sparkbox
 *
 * These functions are the basic functions that should be used to interface with
 * the LCD. Many of these functions were designed for testing and debugging
 * purposes, and are no longer used.
 *
 * Pins in use:
 *
 * | Name | Pin  | Use                  |
 * |------|------|:--------------------:|
 * | FRAME| PA7  | Framerate pin        |
 * | RESET| PA6  | Hardware reset       |
 * | NOE  | PD4  | Read signal line     |
 * | NWE  | PD5  | Write signal line    |
 * | NE1  | PD7  | Chip select line     |
 * | A18  | PD13 | Register select line |
 * | DB0  | PD14 | Data line 0          |
 * | DB1  | PD15 | Data line 1          |
 * | DB2  | PD0  | Data line 2          |
 * | DB3  | PD1  | Data line 3          |
 * | DB4  | PE7  | Data line 4          |
 * | DB5  | PE8  | Data line 5          |
 * | DB6  | PE9  | Data line 6          |
 * | DB7  | PE10 | Data line 7          |
 * | DB8  | PE11 | Data line 8          |
 * | DB9  | PE12 | Data line 9          |
 * | DB10 | PE13 | Data line 10         |
 * | DB11 | PE14 | Data line 11         |
 * | DB12 | PE15 | Data line 12         |
 * | DB13 | PD8  | Data line 13         |
 * | DB14 | PD9  | Data line 14         |
 * | DB15 | PD10 | Data line 15         |
 */

#ifndef SPARK_LCD
#define SPARK_LCD

#include "stm32f4xx.h"
#include "clock.h"

/*! Sets the Reset pin high */
#define LCD_RESET_HIGH	GPIOA->ODR |= (1 << 6)

/*! Sets the Reset pin low */
#define LCD_RESET_LOW	GPIOA->ODR &= ~(1 << 6)

/*! Sets the framerate pin high */
#define LCD_FPS_HIGH	GPIOA->ODR |= (1 << 7)

/*! Sets the framerate pin low */
#define LCD_FPS_LOW		GPIOA->ODR &= ~(1 << 7)

/*! Converts a 24 bit, 888 formatted color to a 16 bit, 565 formatted color */
#define COLOR_888_TO_565(color)	\
	(((color&0xF80000) >> 8) | \
	((color&0x00FC00) >> 5) | \
	((color&0x0000F8) >> 3))

/*! Number of pixels in the LCD width */
#define LCD_WIDTH 320

/*! Number of pixels in the LCD height */
#define LCD_HEIGHT 240

/*! Total number of pixels in the LCD */
#define LCD_PIXELS LCD_WIDTH*LCD_HEIGHT

/*!
 * @name LCD sample colors (565 Format)
 * @{
 */
#define LCD_COLOR_BLUE	 0x001F /*!< Blue */
#define LCD_COLOR_RED	 0xF800 /*!< Red */
#define LCD_COLOR_GREEN	 0x07E0 /*!< Green */
#define LCD_COLOR_BLACK  0x0000 /*!< Black */
#define LCD_COLOR_WHITE  0xFFFF /*!< White */
#define LCD_COLOR_LGRAY  0xE73C /*!< Light Grey */
#define LCD_COLOR_DGRAY  0x18E3 /*!< Dark Grey */
#define LCD_COLOR_YELLOW 0xFFE0 /*!< Yellow */
/* @} */

/*!
 * @brief Enumerable used for LCD commands
 */
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

/*!
 * @brief Initialize LCD
 *
 * Initializes all of the needed peripherals for the LCD, including GPIO 
 * configuration and a hardware initialization for the ILI9341 LCD controller
 */
void initLcd(void);

/*!
 * @brief Write a command to the LCD controller over FSMC
 *
 * @param cmd 16-bits to transfer as a command
 */
void LcdWriteCmd(uint16_t cmd);

/*!
 * @brief Write data to the LCD controller over FSMC
 *
 * @param data 16-bits to transfer as a command parameter or data
 */
void LcdWriteData(uint16_t data);

/*!
 * @brief Read data from the LCD controller over FSMC
 *
 * @return 16-bit data from the LCD
 */
uint16_t LcdReadData(void);

/*!
 * @brief Puts the LCD in sleep mode
 */
void LcdEnterSleep(void);

/*!
 * @brief Removes the LCD from sleep mode
 */
void LcdExitSleep(void);

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
void LcdSetPos(uint16_t x0, uint16_t x1, uint16_t y0, uint16_t y1);

/*!
 * @brief Draws a single pixel at (x, y)
 *
 * @param x x position of the pixel
 * @param y y position of the pixel
 * @param color Color of the pixel to draw, RGB565 format
 */
void LcdPutPixel(uint16_t x, uint16_t y, uint16_t color);

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
uint32_t LcdReadPixel(uint16_t x, uint16_t y);

/*!
 * @brief Fills the whole LCD screen with a single color
 *
 * @param color RGB565 formatted color
 */
void LcdFillScreen(uint16_t color);

/*!
 * @brief Fills the screen with a checkerboard pattern
 */
void LcdFillScreenCheckered(void);

/*!
 * @brief Inverts all colors on the display
 *
 * @param invert Enable (1) or disable (0) the inversion
 */
void LcdInvertDisplay(uint8_t invert);

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
	uint16_t height, uint16_t color);

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
	uint16_t fontColor, uint16_t bgColor);

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
	uint16_t fontColor, uint16_t bgColor);

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
	uint16_t fontColor, uint16_t bgColor);

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
	uint16_t fontColor, uint16_t bgColor);

#endif
