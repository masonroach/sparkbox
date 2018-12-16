/*!
 * @file sprite.c
 * @author Mason Roach
 * @author Patrick Roy
 * @date Dec 13 2018
 *
 * @brief Functions to interact with sprites
 *
 * These functions are the basic functions that should be used to interact with
 * sprites.
 */
#include "sprite.h"

// Static function prototypes
static uint8_t spritesAllocatedAdd(sprite *inSprite);
static uint8_t spritesAllocatedRemove(sprite *inSprite);

/*!
 * @brief Global list to keep track of all initialized sprites
 */
spriteList spritesAllocated = {NULL, 0};

/*!
 * @brief Global list to keep track of sprite layers being shown
 */
spriteList layers = {NULL, 0};

/*!
 * @brief Display helpful debugging information for the given sprite
 *
 * This function displays a debugging screen for a given sprite with information
 * such as width, height, frames, palette, and cycles through each frame of the
 * sprite. Not perfect yet, still does not handle alpha values of the sprite,
 * but still useful for debugging.
 *
 * @param inSprite pointer to the sprite to display
 */
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

/*!
 * @brief Populates a sprite struct
 *
 * Populates the given sprite struct with information from a .spr file from the
 * Sparkbox's SD card.
 *
 * @note This function does not allocate memory for the sprite struct itself. It
 * is up to the user to allocate the memory.
 *
 * @param targetSprite Pointer to a sprite struct with allocated memory
 * @param filename Name of the .spr file on the SD card
 *
 * Example of how to initialize a sprite:
 *
 * @code{.c}
 * . . .
 *
 * sprite testSprite;
 *
 * if (initSprite(&testSprite, "sprite_file.spr")){
 * 	// ERROR
 * 	. . .
 * }
 *
 * . . .
 *
 * @endcode
 */
int8_t initSprite(sprite *targetSprite, char *filename) {
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
		return NOT_ENOUGH_MEMORY;
	}

	// Bytes 7-14 : reserved

	// Fetch 32 bytes of data
	f_read(&targetSprite->file, buffer, 32, NULL);

	// Save the palette from the file to the array and display them
	for (i = 0; i < targetSprite->numColors; i++)
		targetSprite->palette[i] = (buffer[i*2 + 1] << 8) | buffer[i*2];

	return 0;

}

/*!
 * @brief Copy one sprite to another
 *
 * @param inSprite Sprite to copy data from
 * @param targetSprite Sprite to copy data to
 *
 * @warning This function has not been written yet
 */
int8_t copySprite(sprite *inSprite, sprite *targetSprite) {
	// TODO
	return 0;
}

/*!
 * @brief Frees memory allocated by a sprite
 * 
 * Destroys the sprite by freeing memory of the palette and closing the file on
 * the SD card. The sprite is also removed from both spritesAllocated and
 * spriteLayers list.
 * 
 * @param inSprite Sprite struct to destroy
 */
void destroySprite(sprite *inSprite) {

	// Remove from spritesAllocated and spriteLayers lists
	spritesAllocatedRemove(inSprite);
	spriteLayersRemove(inSprite);

	// Close file
	f_close(&inSprite->file);

	// Free memory
	free(inSprite->palette);
	
}

/*!
 * @brief Draw the full sprite on the screen
 * @note This does not work the same way as video, this should only be used for
 * debugging purposes.
 * 
 * @param inSprite Sprite to draw
 * @return Number of pixels drawn
 */
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

/*!
 * @brief Update sprite positions and frames
 *
 * Updates each sprite in the spriteLayers list. The frame of each sprite will
 * increment and shift back to the first sprite once the last frame has been
 * reached. For positions, the x position will move at the speed of the sprite's
 * xvelocity, and the y position will move at the speed of the sprite's
 * yvelocity.
 *
 * @note Only sprites that are at an assigned layer will be updated with this
 * function
 */
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

/*!
 * @brief Set the xpos value of the given sprite
 *
 * @param inSprite Pointer to the sprite struct to change
 * @param x New x value
 */
void spriteSetXpos(sprite *inSprite, int16_t x) {
	inSprite->xpos = x;
}

/*!
 * @brief Set the ypos value of the given sprite
 *
 * @param inSprite Pointer to the sprite struct to change
 * @param y New y value
 */
void spriteSetYpos(sprite *inSprite, int16_t y) {
	inSprite->ypos = y;
}

/*!
 * @brief Set the xpos and ypos value of the given sprite
 *
 * @param inSprite Pointer to the sprite struct to change
 * @param x New x value
 * @param y New y value
 */
void spriteSetPos(sprite *inSprite, int16_t x, int16_t y) {
	inSprite->xpos = x;
	inSprite->ypos = y;
}

/*!
 * @brief Set the flag bits of the given sprite
 *
 * @param inSprite Pointer to the sprite struct to change
 * @param flagVals New flag values
 *
 * @see SPRITE_FLAG
 */
void spriteSetFlags(sprite *inSprite, uint8_t flagVals) {
	inSprite->flags = flagVals;
}

/*!
 * @brief Set or clear the hide flag of the given sprite
 * 
 * @param inSprite Pointer to the sprite struct to change
 * @param hideEnable single bit to either set or clear the HIDE flag
 */
void spriteHide(sprite *inSprite, uint8_t hideEnable) {
	if (hideEnable) {
		inSprite->flags |= HIDE;	// Set the hide flag
	} else {
		inSprite->flags &= ~HIDE;	// Clear the hide flag
	}
}

/*!
 * @brief Set or clear the animate flag of the given sprite
 *
 * @param inSprite Pointer to the sprite struct to change
 * @param animationEnable single bit to either set or clear the ANIMATED flag
 */
void spriteAnimate(sprite *inSprite, uint8_t animationEnable) {
	if (animationEnable) {
		inSprite->flags |= ANIMATED;	// Set the animated flag
	} else {
		inSprite->flags &= ~ANIMATED;	// Clear the animated flag
	}
}

/*!
 * @brief Set a palette color of a sprite to a new given color
 *
 * @param inSprite Pointer to the sprite struct to change
 * @param index Index of the palette color to change, 0 <= index <= 14 
 * @param color New color to place in the palette (RGB565 format)
 */
void spriteSetPaletteColor(sprite *inSprite, uint8_t index, uint16_t color) {
	inSprite->palette[index] = color;
}

/*
 * spritesAllocated functions
 */
/*!
 * @brief Add a sprite pointer to the spritesAllocated list
 *
 * @param inSprite Pointer of sprite to add to the list
 *
 * @return 0 on success, !0 on failure
 */
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

/*!
 * @brief Remove a sprite pointer to the spritesAllocated list
 * 
 * @param inSprite Pointer to the sprite to remove from the list
 *
 * @return 0 on success, !0 on failure
 */
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
/*!
 * @brief Add a sprite pointer to the spriteLayer list at the given position
 *
 * @param inSprite Pointer to the sprite to add to the list
 * @param layer Layer number to add the sprite to
 *
 * @return 0 on success, !0 on failure
 */
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

/*!
 * @brief Append a sprite pointer to the spriteLayer list
 *
 * Adds the sprite to the end of the list, which will be the top-most layer
 * 
 * @param inSprite Pointer to the sprite to add
 *
 * @return 0 on success, !0 on failure
 */
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

/*!
 * @brief Remove the given sprite from the spriteLayers list
 *
 * @param inSprite Pointer to sprite to remove from the list
 *
 * @return 0 on success, !0 on failure
 */
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

/*!
 * @brief For every sprite in the spriteLayers, move their file pointers to
 * beginning of their current frame
 *
 * @return 0 on success, !0 on failure
 */
uint8_t seekStartOfFrames(void) {
	uint32_t offset;
	uint8_t layer;
	uint32_t yAdjust = 0;

	for (layer = 0; layer < layers.size; layer++){
		// Move file pointer to beginning of sprite frame
		offset = ((layers.spr[layer]->width * layers.spr[layer]->height) + 1) / 2;
		
		yAdjust = 0;
		if (layers.spr[layer]->ypos < 0) {
			yAdjust = (layers.spr[layer]->ypos * layers.spr[layer]->width) / 2;
		}

		f_lseek(&layers.spr[layer]->file, 
		 44 + layers.spr[layer]->curFrame * offset - yAdjust);
	}

	return 0;
}
