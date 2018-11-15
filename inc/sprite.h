#ifndef SPARK_SPRITE
#define SPARK_SPRITE

//#include <stdlib.h>
#include <stdint.h>
/*
 * sparksprite file map:
 * Width:       16-bit
 * Height:      16-bit
 * numColors:	4-bit
 * Reserved:	4-bit
 * numFrames:	8-bit
 * Reserved[4]: 16-bits each
 * Palette[16]: 16-bits each
 * ColorMap[x]: 4-bits each
 */

typedef struct sprite {

	/* Need some kind of file pointer */

	uint16_t width;
	uint16_t height;
	uint8_t numFrames;
	uint8_t numColors;
	uint16_t palette[15];
} sprite;

typedef enum {
	NOT_ENOUGH_MEMORY = 1,
	
} SPRITE_ERROR;

#define SAMPLE_SPRITE 4
#if SAMPLE_SPRITE>0
#include "lcd.h"
void drawSprite(sprite *inSprite);
uint8_t test_getSprite(sprite *inSprite);
uint16_t test_get16(void);
uint16_t test_fseek(int32_t offset, uint8_t whence);

typedef enum {
	TEST_SEEK_SET = 0,
	TEST_SEEK_CUR = 1,
	TEST_SEEK_END = 2
} FSEEK_WHENCE;
#endif

#endif
