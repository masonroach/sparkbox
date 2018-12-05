#ifndef SPARKBOX_VIDEO
#define SPARKBOX_VIDEO

#include "stm32f4xx.h"
#include "ff.h"
#include "lcd.h"
#include "sprite.h"
#include "waveplayer.h"
#include "led.h"

// Size of LCD in bytes, not pixels
#define LCD_SIZE_BYTES (LCD_PIXELS * 2)

// Number of rows per transfer
// NOTE: Use a multiple of 4
#define LCD_TRANSFER_ROWS 12

// Number of pixels in 4 rows
#define FOUR_ROW_OFFSET LCD_WIDTH*4

// Number of buffer transfers to fill LCD
#define NUM_TRANSFERS (LCD_HEIGHT / LCD_TRANSFER_ROWS)

// Number of transfers is not 
#if (LCD_SIZE_BYTES % NUM_TRANSFERS)
#error "LCD_TRANSFER_ROWS must evenly divide LCD_HEIGHT."
#endif

// Video buffer size is 5.120kB
#define VID_BUF_BYTES (LCD_SIZE_BYTES / NUM_TRANSFERS)

// Timer 1 frequency
#define TIM1FREQ 84000000UL

// Address to write pixel data
#define LCD_DATA_ADDR 0x60080000UL

// Default background color
#define VIDEO_BG COLOR_888_TO_565(0x00A591)

// PSC 99 ARR 299
#define TIM7PSC 99
#define TIM7ARR 299

extern volatile uint16_t * const fsmc_data;
extern spriteList spriteLayers;

int8_t initVideo(void);
void updateFrame(void);
uint8_t testGetRow(const uint8_t row);
uint8_t getNext4Rows(uint8_t set);

#endif
