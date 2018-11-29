#ifndef SPARK_SPRITE
#define SPARK_SPRITE

#include <stdlib.h>
#include <stdint.h>
#include "ff.h"

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

typedef struct {
	FIL file;
	uint16_t width;
	uint16_t height;
	uint16_t xpos;
	uint16_t ypos;
	uint16_t numColors;
	uint16_t *palette;
	uint8_t numFrames;
	uint8_t curFrame;
	uint8_t flags;
} sprite;

typedef enum {
	HIDE = 0x80,
	ANIMATED = 0x40,
//	reserved = 0x20,
//	reserved = 0x10,
//	reserved = 0x08,
//	reserved = 0x04,
//	reserved = 0x02,
//	reserved = 0x01
} SPRITE_FLAG;

typedef enum {
	NOT_ENOUGH_MEMORY = 1,
	NO_FILE_ACCESS = 2,
	
} SPRITE_ERROR;

uint32_t drawSprite(sprite *inSprite);

#define SAMPLE_SPRITE 6
#if SAMPLE_SPRITE>0
#include "lcd.h"
#include "sparkboxButtons.h"
void drawSpriteDebug(sprite *inSprite);
sprite *test_getSprite(void);
uint16_t test_get16(void);
uint16_t test_fseek(int32_t offset, uint8_t whence);

typedef enum {
	TEST_SEEK_SET = 0,
	TEST_SEEK_CUR = 1,
	TEST_SEEK_END = 2
} FSEEK_WHENCE;
#endif

#endif
