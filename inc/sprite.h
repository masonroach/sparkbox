#ifndef SPARK_SPRITE
#define SPARK_SPRITE

#include <stdlib.h>
#include <stdint.h>
#include "ff.h"
#include "lcd.h"
#include "button.h"
#include "led.h"

#define MAX_LAYERS 8
#define MAX_SPRITES 32

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

typedef struct sprite_struct{
	FIL file;
	uint16_t width;
	uint16_t height;
	int16_t xpos;
	int16_t ypos;
	int16_t xvelocity;
	int16_t yvelocity;
	uint16_t numColors;
	uint16_t *palette;
	uint8_t numFrames;
	uint8_t curFrame;
	uint8_t flags;
	uint8_t tag;
	int8_t layer;
} sprite;

typedef struct {
	sprite **spr;
	uint8_t size;
} spriteList;

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
	TOO_MANY_SPRITES = 3,
	FILE_ERROR = 4,
} SPRITE_ERROR;

// sprite functions
int8_t initSprite(sprite *targetSprite, TCHAR *filename);
int8_t copySprite(sprite *inSprite, sprite *targetSprite);
uint32_t drawSprite(sprite *inSprite);
void destroySprite(sprite *inSprite);
void updateSprites(void);
void spriteSetXpos(sprite *inSprite, int16_t x);
void spriteSetYpos(sprite *inSprite, int16_t y);
void spriteSetPos(sprite *inSprite, int16_t x, int16_t y);
void spriteSetFlags(sprite *inSprite, uint8_t flagVals);
void spriteHide(sprite *inSprite, uint8_t hideEnable);
void spriteAnimate(sprite *inSprite, uint8_t animationEnable);
void spriteSetPaletteColor(sprite *inSprite, uint8_t num, uint16_t color);

// spriteLayers functions
uint8_t spriteLayersInsert(sprite *inSprite, uint8_t layer);
uint8_t spriteLayersAdd(sprite *inSprite);
uint8_t spriteLayersRemove(sprite *inSprite);
uint8_t seekStartOfFrames(void);

// test functions
void drawSpriteDebug(sprite *inSprite);

#endif
