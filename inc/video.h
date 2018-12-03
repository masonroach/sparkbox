#ifndef SPARKBOX_VIDEO
#define SPARKBOX_VIDEO

#include "stm32f4xx.h"
#include "ff.h"
#include "lcd.h"
#include "sprite.h"
#include "waveplayer.h"

// Size of LCD in bytes, not pixels
#define LCD_SIZE_BYTES (LCD_PIXELS * 2)

// Video buffer size is 6.4kB
#define VID_BUF_BYTES (6400)

// Number of buffer transfers to fill LCD (24)
#define NUM_TRANSFERS (LCD_SIZE_BYTES / VID_BUF_BYTES)

// Timer 7 frequency
#define TIM7FREQ 84000000UL

// Address to write pixel data
#define LCD_DATA_ADDR 0x60080000UL

// Default background color
#define VIDEO_BG LCD_COLOR_BLACK

// LCD writes no less than 186 nS apart
// LCD write frequency no greater than 5.376 MHz
// PSC of 15 -> LCD write feq = 84 MHz / (1 + 15) = 5.25 MHz
#define TIM7PSC 15

// The filename of the frame buffer on SD card
#define FRAME_FILE "mason.bin"

extern volatile uint16_t * const fsmc_data;

void initVideo(void);
void updateFrame(void);

#endif
