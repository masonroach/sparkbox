#include "sprite.h"

// Static functions
static uint8_t spritesAllocatedAdd(sprite *inSprite);
static uint8_t spritesAllocatedRemove(sprite *inSprite);

// Global list to keep track of all initialized sprites
spriteList spritesAllocated = {NULL, 0};

// Global list to keep track of sprite layers being shown
spriteList layers = {NULL, 0};

// Display helpful debugging information for the given sprite
void drawSpriteDebug(sprite *inSprite) {
	uint16_t i;

	LcdFillScreenCheckered();
	LcdDrawRectangle(240, 0, 80, 240, LCD_COLOR_BLACK);

	// Write headers to the screen
	LcdDrawString(250, 2, (uint8_t *)"W =", LCD_COLOR_WHITE, LCD_COLOR_BLACK);
	LcdDrawString(250, 14, (uint8_t *)"H =", LCD_COLOR_WHITE, LCD_COLOR_BLACK);
	LcdDrawString(250, 26, (uint8_t *)"F =", LCD_COLOR_WHITE, LCD_COLOR_BLACK);
	LcdDrawString(250, 50, (uint8_t *)"PALETTE", LCD_COLOR_WHITE, LCD_COLOR_BLACK);

	LcdDrawInt(278, 2, inSprite->width, LCD_COLOR_WHITE, LCD_COLOR_BLACK);
	LcdDrawInt(278, 14, inSprite->height, LCD_COLOR_WHITE, LCD_COLOR_BLACK);
	LcdDrawInt(278, 26, inSprite->numFrames, LCD_COLOR_WHITE, LCD_COLOR_BLACK);

	// Display palette colors
	for (i = 0; i < inSprite->numColors; i++) {
		LcdDrawRectangle(245, 62 + 12*i, 10, 10, inSprite->palette[i]);
		LcdDrawInt(260, 62 + 12*i, i+1, LCD_COLOR_WHITE, LCD_COLOR_BLACK);
	}

	// Set sprite position to middle of screen
	inSprite->xpos = 120 - inSprite->width/2;
	inSprite->ypos = 120 - inSprite->height/2;

	// Draw the sprite
	while (!readButton()) {
		// Update frame number
		LcdDrawRectangle(278, 26, 40, 10, LCD_COLOR_BLACK);
		LcdDrawInt(278, 26, inSprite->curFrame, LCD_COLOR_WHITE, LCD_COLOR_BLACK);

		// Draw the sprite
		drawSprite(inSprite);

		// TEMPORARY UNTIL VIDEO WORKS 
		// Go to next frame after delay
		delayms(50);
		inSprite->curFrame++;
		if (inSprite->curFrame == inSprite->numFrames) inSprite->curFrame = 0;
	}
	
}

/*
 te functions
 */
// Create a sprite struct from the given filename
int8_t initSprite(sprite *targetSprite, TCHAR *filename) {
	uint8_t buffer[32];
	uint16_t i;

	// Open the sprite file
	if (f_open(&targetSprite->file, filename, FA_READ) != FR_OK) {
		// If file open failed,
		return NO_FILE_ACCESS;
	}

	// Get the tag for the sprite
	if (spritesAllocatedAdd(targetSprite)) {
		// If failed, free memory and break
		f_close(&targetSprite->file);
		return TOO_MANY_SPRITES;
	}

	// Initialize data that is not in file
	targetSprite->xpos = 0;
	targetSprite->ypos = 0;
	targetSprite->xvelocity = 0;
	targetSprite->yvelocity = 0;
	targetSprite->curFrame = 0;
	targetSprite->flags = 0x00;
	targetSprite->layer = -1;

	// Get header data
	f_read(&targetSprite->file, buffer, 14, NULL);	// Fetch 16 bytes of data
	targetSprite->width = (buffer[1] << 8) | buffer[0];	// 2 bytes : width
	targetSprite->height = (buffer[3] << 8) | buffer[2];	// 2 bytes : height
	targetSprite->numFrames = buffer[4];	// 1 byte : numFrames
	targetSprite->numColors = (buffer[5] & 0x00F0) >> 4;	// 4 bits : nColors
	// targetSprite->________ = buffer[5] & 0x000F;	// 4 bits : Reserved

	// Allocate palette array
	targetSprite->palette = (uint16_t *)malloc(
		(targetSprite->numColors) * sizeof(uint16_t));
	if (targetSprite->palette == NULL) {
		spritesAllocatedRemove(targetSprite);
		f_close(&targetSprite->file);
		return NULL;
	}

	// Bytes 7-14 : reserved

	// Fetch 32 bytes of data
	f_read(&targetSprite->file, buffer, 32, NULL);

	// Save the palette from the file to the array and display them
	for (i = 0; i < targetSprite->numColors; i++)
		targetSprite->palette[i] = (buffer[i*2 + 1] << 8) | buffer[i*2];

	return 0;

}

// Copy one sprite to another
int8_t copySprite(sprite *inSprite, sprite *targetSprite) {
	// TODO
	return 0;
}

// Frees memory allocated by a sprite
void destroySprite(sprite *inSprite) {

	// Remove from spritesAllocated and spriteLayers lists
	spritesAllocatedRemove(inSprite);
	spriteLayersRemove(inSprite);

	// Close file
	f_close(&inSprite->file);

	// Free memory
	free(inSprite->palette);
	
}

// Draw the full sprite on the screen. Note: this does not work the same way as
// video, this is mostly for debugging purposes.
uint32_t drawSprite(sprite *inSprite) {
	uint8_t temp;
	uint32_t i;
	uint32_t offset;

	// Find the offset of each frame for the file pointer
	// If width*height is not a multiple of 2, rounds up before dividing
	offset = ((inSprite->width * inSprite->height) + 1) / 2;

	// Move file pointer to beginning of sprite frame
	// Number of bytes to skip:
	// 2 bytes  : width
	// 2 bytes  : height
	// 1 byte   : numFrames
	// 1 byte   : numColors
	// 8 bytes  : reserved
	// 30 bytes : palette
	f_lseek(&inSprite->file, 44 + inSprite->curFrame*offset);

	// Set drawing window on the LCD
	LcdSetPos(inSprite->xpos, inSprite->ypos, inSprite->xpos + inSprite->width-1, inSprite->ypos + inSprite->height-1);
	LcdWriteCmd(MEMORY_WRITE);

	// Write the rest of the pixels
	for (i = 0 ; i < (inSprite->width * inSprite->height) - 2; i+=2) {
		f_read(&inSprite->file, &temp, 1, NULL);	// Get 2 pixels of data
		LcdWriteData(inSprite->palette[(temp & 0x0F)-1]);	// Draw a pixel
		LcdWriteData(inSprite->palette[((temp & 0xF0)>>4)-1]);	// Draw a pixel
	}
	// Get tail set of pixels

	return i;

}

// Update sprite positions and frames
void updateSprites(void) {
	uint8_t layer;
	
	for (layer = 0; layer < layers.size; layer++) {

		// Update frames
		layers.spr[layer]->curFrame++;
		if (layers.spr[layer]->curFrame == layers.spr[layer]->numFrames) {
			layers.spr[layer]->curFrame = 0;
		}

		// Update positions
		layers.spr[layer]->xpos += layers.spr[layer]->xvelocity;
		layers.spr[layer]->ypos += layers.spr[layer]->yvelocity;

	}

}

// Set the xpos value of the given sprite
void spriteSetXpos(sprite *inSprite, int16_t x) {
	inSprite->xpos = x;
}

// Set the ypos value of the given sprite
void spriteSetYpos(sprite *inSprite, int16_t y) {
	inSprite->ypos = y;
}

// Set the xpos and ypos value of the given sprite
void spriteSetPos(sprite *inSprite, int16_t x, int16_t y) {
	inSprite->xpos = x;
	inSprite->ypos = y;
}

// Set the flag bits of the given sprite
// X0000000 Hide: if set to 1, will not draw the sprite on the LCD
// 0X000000 Animated: if set to 1, the sprite will cycle through frames
// 00X00000 Reserved
// 000X0000 Reserved
// 0000X000 Reserved
// 00000X00 Reserved
// 000000X0 Reserved
// 0000000X Reserved
void spriteSetFlags(sprite *inSprite, uint8_t flagVals) {
	inSprite->flags = flagVals;
}

// Set or clear the hide flag of the given sprite
void spriteHide(sprite *inSprite, uint8_t hideEnable) {
	if (hideEnable) {
		inSprite->flags |= HIDE;	// Set the hide flag
	} else {
		inSprite->flags &= ~HIDE;	// Clear the hide flag
	}
}

// Set or clear the animate flag of the given sprite
void spriteAnimate(sprite *inSprite, uint8_t animationEnable) {
	if (animationEnable) {
		inSprite->flags |= ANIMATED;	// Set the animated flag
	} else {
		inSprite->flags &= ~ANIMATED;	// Clear the animated flag
	}
}

// Set a palette color of a sprite to a new given color
void spriteSetPaletteColor(sprite *inSprite, uint8_t num, uint16_t color) {
	inSprite->palette[num] = color;
}

/*
 * spritesAllocated functions
 */
// Add a sprite pointer to the spritesAllocated list
// Return 0 on success, !0 on failure
static uint8_t spritesAllocatedAdd(sprite *inSprite) {
	sprite **newPointer;

	// Check if too many sprites are already allocated
	if (spritesAllocated.size >= MAX_SPRITES) {
		return TOO_MANY_SPRITES;
	}
	
	// Reallocate memory for array of structs
	newPointer = (sprite **)realloc(spritesAllocated.spr, spritesAllocated.size+1 * sizeof(sprite *));
	if (newPointer == NULL) {
		return NOT_ENOUGH_MEMORY;
	}
	spritesAllocated.spr = newPointer;
	
	// Put the sprite at the end of the array
	spritesAllocated.spr[spritesAllocated.size] = inSprite;

	// Keep track of tag in inSprite
	inSprite->tag = spritesAllocated.size;

	// Increment size to reflect new size	
	spritesAllocated.size++;

	return 0;
}

// Add a sprite pointer to the spritesAllocated list
// Return 0 on success, !0 on failure
static uint8_t spritesAllocatedRemove(sprite *inSprite) {
	uint8_t i;
	
	// Remove the element and move the rest of the elements back
	for (i = inSprite->tag; i < spritesAllocated.size - 1; i++) {
		spritesAllocated.spr[i] = spritesAllocated.spr[i + 1];
	}

	// Decrement size
	spritesAllocated.size--;

	return 0;

}

/*
 * spriteLayer functions
 */
// Add a sprite pointer to the spriteLayer list at the given position
// Return 0 on success, !0 on failure
uint8_t spriteLayersInsert(sprite *inSprite, uint8_t layer) {
	uint8_t i;
	sprite **newPointer;

	// Check if too many sprites are already allocated
	if (layers.size >= MAX_LAYERS) {
		return TOO_MANY_SPRITES;
	}
	
	// Reallocate memory for array of structs
	newPointer = (sprite **)realloc(layers.spr, layers.size+1 * sizeof(sprite *));
	if (newPointer == NULL) {
		return NOT_ENOUGH_MEMORY;
	}
	layers.spr = newPointer;
	
	// Move the layers after the given index down
	for (i = spritesAllocated.size; i > layer + 1; i--) {
		layers.spr[i] = layers.spr[i-1];
	}

	// Insert the sprite at the given index
	layers.spr[layer] = inSprite;

	// Keep track of tag in inSprite
	inSprite->layer = layer;

	// Increment size to reflect new size	
	layers.size++;

	return 0;
}

// Append a sprite pointer to the spriteLayer list
// Return 0 on success, !0 on failure
uint8_t spriteLayersAdd(sprite *inSprite) {
	sprite **newPointer;

	// Check if too many sprites are already allocated
	if (layers.size >= MAX_LAYERS) {
		return TOO_MANY_SPRITES;
	}
	
	// Reallocate memory for array of structs
	newPointer = (sprite **)realloc(layers.spr, layers.size+1 * sizeof(sprite *));
	if (newPointer == NULL) {
		return NOT_ENOUGH_MEMORY;
	}
	layers.spr = newPointer;
	
	// Insert the sprite at the given index
	layers.spr[layers.size] = inSprite;

	// Keep track of tag in inSprite
	inSprite->layer = layers.size;

	// Increment size to reflect new size	
	layers.size++;

	return 0;
}

// Remove the given sprite from the spriteLayers list
// Return 0 on success, !0 on failure
uint8_t spriteLayersRemove(sprite *inSprite) {
	uint8_t i;
	
	// Remove the element and move the rest of the elements back
	for (i = inSprite->layer; i < layers.size - 1; i++) {
		layers.spr[i] = layers.spr[i + 1];
	}

	// Decrement size
	layers.size--;

	// Set the layers flag in the sprite
	inSprite->layer = -1;

	return 0;
}

// For every sprite in the layers, move their file pointers to beginning of
// their current frame
uint8_t seekStartOfFrames(void) {
	uint32_t offset;
	uint8_t layer;

	for (layer = 0; layer < layers.size; layer++){

		// Move file pointer to beginning of sprite frame
		offset = ((layers.spr[layer]->width * layers.spr[layer]->height) + 1) / 2;
		f_lseek(&layers.spr[layer]->file, 44 + layers.spr[layer]->curFrame * offset);
	}

	return 0;
}
