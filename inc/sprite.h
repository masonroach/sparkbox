#ifndef SPARK_SPRITE
#define SPARK_SPRITE

#include "lcd.h"

/*
 * sparksprite file map:
 * Width:       16-bit
 * Height:      16-bit
 * nColors:		16-bit
 * Reserved[4]: 16-bits each
 * Palette[16]: 16-bits each
 * ColorMap[x]: 4-bits each
 */

#define SAMPLE_SPRITE 2
#if SAMPLE_SPRITE>0
void test_drawSprite(void);
uint16_t test_get16(void);
#endif

void drawSprite();

#endif
