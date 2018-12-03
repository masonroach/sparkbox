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

// Video buffer size is 6.4kB
#define VID_BUF_BYTES (6400)

// Number of buffer transfers to fill LCD (24)
#define NUM_TRANSFERS 24

// Timer 1 frequency
#define TIM1FREQ 84000000UL

// Address to write pixel data
#define LCD_DATA_ADDR 0x60080000UL

// Default background color
#define VIDEO_BG LCD_COLOR_BLACK

// The filename of the frame buffer on SD card
#define FRAME_FILE "mason.bin"

// PSC 99 ARR 299
#define TIM7PSC 99
#define TIM7ARR 299

extern volatile uint16_t * const fsmc_data;

int8_t initVideo(void);
void updateFrame(void);

#endif
