#include "sprite.h"

// Static functions
static uint8_t spritesAllocatedAdd(sprite inSprite);
static uint8_t spritesAllocatedRemove(sprite inSprite);

// Global list to keep track of all initialized sprites
spriteList spritesAllocated = {NULL, 0};

// Global list to keep track of sprite layers being shown
spriteList spriteLayers = {NULL, 0};


#if SAMPLE_SPRITE>0
extern const uint16_t fakeSpriteFile[];

// Global file pointer for test sprite file
static uint32_t filePointer = 0;

// Test assembling a sprite from a dummy file
sprite test_getSprite(void) {
	uint16_t temp, i;
	sprite targetSprite;

	// Allocate space for sprite
	targetSprite = (sprite )malloc(sizeof(sprite));
	if (targetSprite == NULL) {
		return NULL;
	}

	// Get the tag for the sprite
	if (spritesAllocatedAdd(targetSprite)) {
		// If failed, free memory and break
		free(targetSprite);
		return NULL;
	}

	// Initialize data that is not in file
	targetSprite->xpos = 0;
	targetSprite->ypos = 0;
	targetSprite->xvelocity = 0;
	targetSprite->yvelocity = 0;
	targetSprite->curFrame = 0;
	targetSprite->flags = 0x00;
	targetSprite->layer = -1;

	/******************/
	/* OPEN FILE HERE */
	/******************/

	// Move pointer to beginning of file
	filePointer = 0;

	// Get header data
	temp = test_get16();
	targetSprite->width = temp;	// half-word1 : width
	temp = test_get16();
	targetSprite->height = temp;	// half-word2 : height
	temp = test_get16();	// half-word3 : numFrames, 4 reserved, numColors
	targetSprite->numFrames = (temp & 0xFF00) >> 8;	// numFrames
	// targetSprite->________ = (temp & 0x00F0) >> 4);	// Reserved
	targetSprite->numColors = temp & 0x000F;	// numColors

	// Allocate palette array
	targetSprite->palette = (uint16_t *)malloc(
		(targetSprite->numColors) * sizeof(uint16_t));
	if (targetSprite->palette == NULL) {
		spritesAllocatedRemove(targetSprite);
		free(targetSprite);
		return NULL;
	}

	test_get16();	// half-word4 : Reserved
	test_get16();	// half-word5 : Reserved
	test_get16();	// half-word6 : Reserved
	test_get16();	// half-word7 : Reserved

	// Save the palette from the file to the array and display them
	for (i = 0; i < targetSprite->numColors; i++)
		targetSprite->palette[i] = test_get16();

	// Throw away the rest of the palette
	for ( ; i < 15; i++) test_get16();

	return targetSprite;

}

// Display helpful debugging information for the given sprite
void drawSpriteDebug(sprite inSprite) {
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
	LcdDrawInt(278, 26, inSprite->curFrame, LCD_COLOR_WHITE, LCD_COLOR_BLACK);
/*
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
*/	
}

// Get the next value in the test array
uint16_t test_get16(void) {
	uint16_t r = fakeSpriteFile[filePointer];
	filePointer++;
	return r;
}

// Change the file pointer to a location
uint16_t test_fseek(int32_t offset, uint8_t whence) {
	switch (whence) {
		case TEST_SEEK_SET:
			filePointer = offset;
			break;
		case TEST_SEEK_CUR:
			filePointer += offset;
			break;
		case TEST_SEEK_END:
			// Not implemented. Can't find EOF with just an array and I don't need it
			break;
		default:
			break;
	}

	return filePointer;

}
#endif

/*
 * sprite functions
 */
// Create a sprite struct from the given filename
sprite initSprite(const char *filename) {
	FIL *file = NULL;
	FRESULT res;
	uint8_t buffer[32];
	uint16_t i;
	sprite targetSprite;

	ledOn(1);
	// Open the sprite file
	res = f_open(file, filename, FA_READ);
	if (res != FR_OK) {
		// If file open failed,
		return NULL;
	}

	// Allocate space for sprite
	ledOn(2);
	targetSprite = (sprite)malloc(sizeof(sprite));
	if (targetSprite == NULL) {
		f_close(file);
		return NULL;
	}

	// Get the tag for the sprite
		ledOn(3);
	if (spritesAllocatedAdd(targetSprite)) {
		// If failed, free memory and break
		f_close(file);
		free(targetSprite);
		return NULL;
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
	f_read(file, buffer, 14, NULL);	// Fetch 16 bytes of data
	targetSprite->width = (buffer[0] << 8) | buffer[1];	// 16 bits : width
	targetSprite->height = (buffer[2] << 8) | buffer[3];	// 16 bits : height
	targetSprite->numFrames = buffer[5];	// 8 bits : numFrames
	// targetSprite->________ = (buffer[6] & 0x00F0) >> 4);	// Reserved
	targetSprite->numColors = buffer[6] & 0x000F;	// 4 bits : numColors

	// Allocate palette array
	targetSprite->palette = (uint16_t *)malloc(
		(targetSprite->numColors) * sizeof(uint16_t));
	if (targetSprite->palette == NULL) {
		spritesAllocatedRemove(targetSprite);
		f_close(file);
		free(targetSprite);
		return NULL;
	}

	// Bytes 7-14 : reserved

	// Fetch 32 bytes of data
	f_read(file, buffer, 32, NULL);

	// Save the palette from the file to the array and display them
	for (i = 0; i < targetSprite->numColors; i++)
		targetSprite->palette[i] = (buffer[i*2] << 8) | buffer[i*2 + 1];

	return targetSprite;

}

// Copy one sprite to another
sprite copySprite(sprite const inSprite) {
	// TODO
	return NULL;
}

// Frees memory allocated by a sprite
void destroySprite(sprite inSprite) {

	// Remove from spritesAllocated and spriteLayers lists
	spritesAllocatedRemove(inSprite);
	spriteLayersRemove(inSprite);

	// Close file
	f_close(inSprite->file);

	// Free memory
	free(inSprite->palette);
	free(inSprite);
	
}

// Draw the full sprite on the screen. Note: this does not work the same way as
// video, this is mostly for debugging purposes.
uint32_t drawSprite(sprite inSprite) {
	uint16_t temp;
	uint32_t i = 0, p;
	uint32_t offset;

	// Find the offset of each frame for the file pointer
	// If width*height is not a multiple of 4, rounds up before dividing
	offset = ((inSprite->width * inSprite->height) + 3) / 4;

	// Move file pointer to beginning of sprite frame
	test_fseek(22 + inSprite->curFrame*offset, TEST_SEEK_SET);

	// Set drawing window on the LCD
	LcdSetPos(inSprite->xpos, inSprite->ypos, inSprite->xpos + inSprite->width-1, inSprite->ypos + inSprite->height-1);
	LcdWriteCmd(MEMORY_WRITE);

	// Write the rest of the pixels
	for ( ; i < (inSprite->width * inSprite->height) - 4; i+=4) {
		temp = test_get16();	// Get 4 pixels worth of data
		LcdWriteData(inSprite->palette[(temp & 0x000F)-1]);	// Draw a pixel
		LcdWriteData(inSprite->palette[((temp & 0x00F0)>>4)-1]);	// Draw a pixel
		LcdWriteData(inSprite->palette[((temp & 0x0F00)>>8)-1]);	// Draw a pixel
		LcdWriteData(inSprite->palette[((temp & 0xF000)>>12)-1]);	// Draw a pixel
	}

	// Get tail set of pixels
	temp = test_get16();	// Get 4 pixels worth of data
	p = 0;
	while (i < (inSprite->width * inSprite->height)) {
		LcdWriteData(inSprite->palette[((temp & (0x000F << (p*4)))>>(p*4))-1]);
		p++;
		i++;
	}

	return i;

}

// Update sprite positions and frames
void updateSprites(void) {
	uint8_t layer;
	
	for (layer = 0; layer < spriteLayers.size; layer++) {

		// Update frames
		spriteLayers.sprites[layer]->curFrame++;
		if (spriteLayers.sprites[layer]->curFrame == spriteLayers.sprites[layer]->numFrames) {
			spriteLayers.sprites[layer]->curFrame = 0;
		}

		// Update positions
		spriteLayers.sprites[layer]->xpos += spriteLayers.sprites[layer]->xvelocity;
		spriteLayers.sprites[layer]->ypos += spriteLayers.sprites[layer]->yvelocity;

	}

}

// Set the xpos value of the given sprite
void spriteSetXpos(sprite inSprite, int16_t x) {
	inSprite->xpos = x;
}

// Set the ypos value of the given sprite
void spriteSetYpos(sprite inSprite, int16_t y) {
	inSprite->ypos = y;
}

// Set the xpos and ypos value of the given sprite
void spriteSetPos(sprite inSprite, int16_t x, int16_t y) {
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
void spriteSetFlags(sprite inSprite, uint8_t flagVals) {
	inSprite->flags = flagVals;
}

// Set or clear the hide flag of the given sprite
void spriteHide(sprite inSprite, uint8_t hideEnable) {
	if (hideEnable) {
		inSprite->flags |= HIDE;	// Set the hide flag
	} else {
		inSprite->flags &= ~HIDE;	// Clear the hide flag
	}
}

// Set or clear the animate flag of the given sprite
void spriteAnimate(sprite inSprite, uint8_t animationEnable) {
	if (animationEnable) {
		inSprite->flags |= ANIMATED;	// Set the animated flag
	} else {
		inSprite->flags &= ~ANIMATED;	// Clear the animated flag
	}
}

// Set a palette color of a sprite to a new given color
void spriteSetPaletteColor(sprite inSprite, uint8_t num, uint16_t color) {
	inSprite->palette[num] = color;
}

/*
 * spritesAllocated functions
 */
// Add a sprite pointer to the spritesAllocated list
// Return 0 on success, !0 on failure
static uint8_t spritesAllocatedAdd(sprite inSprite) {
	sprite *newPointer;

	// Check if too many sprites are already allocated
	if (spritesAllocated.size >= MAX_SPRITES) {
		return TOO_MANY_SPRITES;
	}
	
	// Reallocate memory for array of structs
	newPointer = (sprite *)realloc(spritesAllocated.sprites, spritesAllocated.size+1 * sizeof(sprite));
	if (newPointer == NULL) {
		return NOT_ENOUGH_MEMORY;
	}
	spritesAllocated.sprites = newPointer;
	
	// Put the sprite at the end of the array
	spritesAllocated.sprites[spritesAllocated.size] = inSprite;

	// Keep track of tag in inSprite
	inSprite->tag = spritesAllocated.size;

	// Increment size to reflect new size	
	spritesAllocated.size++;

	return 0;
}

// Add a sprite pointer to the spritesAllocated list
// Return 0 on success, !0 on failure
static uint8_t spritesAllocatedRemove(sprite inSprite) {
	uint8_t i;
	
	// Remove the element and move the rest of the elements back
	for (i = inSprite->tag; i < spritesAllocated.size - 1; i++) {
		spritesAllocated.sprites[i] = spritesAllocated.sprites[i + 1];
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
uint8_t spriteLayersInsert(sprite inSprite, uint8_t layer) {
	uint8_t i;
	sprite *newPointer;

	// Check if too many sprites are already allocated
	if (spriteLayers.size >= MAX_LAYERS) {
		return TOO_MANY_SPRITES;
	}
	
	// Reallocate memory for array of structs
	newPointer = (sprite *)realloc(spriteLayers.sprites, spriteLayers.size+1 * sizeof(sprite));
	if (newPointer == NULL) {
		return NOT_ENOUGH_MEMORY;
	}
	spriteLayers.sprites = newPointer;
	
	// Move the layers after the given index down
	for (i = spritesAllocated.size; i > layer + 1; i--) {
		spriteLayers.sprites[i] = spriteLayers.sprites[i-1];
	}

	// Insert the sprite at the given index
	spriteLayers.sprites[layer] = inSprite;

	// Keep track of tag in inSprite
	inSprite->layer = layer;

	// Increment size to reflect new size	
	spriteLayers.size++;

	return 0;
}

// Append a sprite pointer to the spriteLayer list
// Return 0 on success, !0 on failure
uint8_t spriteLayersAdd(sprite inSprite) {
	sprite *newPointer;

	// Check if too many sprites are already allocated
	if (spriteLayers.size >= MAX_LAYERS) {
		return TOO_MANY_SPRITES;
	}
	
	// Reallocate memory for array of structs
	newPointer = (sprite *)realloc(spriteLayers.sprites, spriteLayers.size+1 * sizeof(sprite));
	if (newPointer == NULL) {
		return NOT_ENOUGH_MEMORY;
	}
	spriteLayers.sprites = newPointer;
	
	// Insert the sprite at the given index
	spriteLayers.sprites[spriteLayers.size] = inSprite;

	// Keep track of tag in inSprite
	inSprite->layer = spriteLayers.size;

	// Increment size to reflect new size	
	spriteLayers.size++;

	return 0;
}

// Remove the given sprite from the spriteLayers list
// Return 0 on success, !0 on failure
uint8_t spriteLayersRemove(sprite inSprite) {
	uint8_t i;
	
	// Remove the element and move the rest of the elements back
	for (i = inSprite->layer; i < spriteLayers.size - 1; i++) {
		spriteLayers.sprites[i] = spriteLayers.sprites[i + 1];
	}

	// Decrement size
	spriteLayers.size--;

	// Set the layers flag in the sprite
	inSprite->layer = -1;

	return 0;
}

// For every sprite in the layers, move their file pointers to beginning of
// their current frame
uint8_t seekStartOfFrames(void) {
	uint32_t offset;
	uint8_t layer;

	for (layer = 0; layer < spriteLayers.size; layer++){

		// Move file pointer to beginning of sprite frame
		offset = ((spriteLayers.sprites[layer]->width * spriteLayers.sprites[layer]->height) + 3) / 4;
		test_fseek(22 + spriteLayers.sprites[layer]->curFrame * offset, TEST_SEEK_SET);
	}

	return 0;
}
