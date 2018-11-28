#include "sprite.h"

#if SAMPLE_SPRITE>0
extern const uint16_t fakeSpriteFile[];

// Global file pointer for test sprite file
static uint32_t filePointer = 0;

// Test assembling a sprite from a dummy file
sprite *test_getSprite(void) {
	uint16_t temp, i;
	sprite *targetSprite;

	// Allocate space for sprite
	targetSprite = (sprite *)malloc(sizeof(sprite));
	if (targetSprite == NULL) {
		return NULL;
	}

	// Initialize data that is not in file
	targetSprite->xpos = 0;
	targetSprite->ypos = 0;
	targetSprite->curFrame = 0;
	targetSprite->flags = 0x00;

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
void drawSpriteDebug(sprite *inSprite) {
	uint16_t i;

	LcdFillScreenCheckered();
	LcdDrawRectangle(240, 0, 80, 240, LCD_COLOR_BLACK);

	// Write headers to the screen
	LcdDrawString(250, 2, (uint8_t *)"W =", LCD_COLOR_WHITE, LCD_COLOR_BLACK);
	LcdDrawString(250, 14, (uint8_t *)"H =", LCD_COLOR_WHITE, LCD_COLOR_BLACK);
	LcdDrawString(250, 26, (uint8_t *)"F =", LCD_COLOR_WHITE, LCD_COLOR_BLACK);
	//LcdDrawString(250, 38, (uint8_t *)"RESERVED", LCD_COLOR_WHITE, LCD_COLOR_BLACK);
	LcdDrawString(250, 50, (uint8_t *)"PALETTE", LCD_COLOR_WHITE, LCD_COLOR_BLACK);
	LcdDrawInt(278, 2, inSprite->width, LCD_COLOR_WHITE, LCD_COLOR_BLACK);
	LcdDrawInt(278, 14, inSprite->height, LCD_COLOR_WHITE, LCD_COLOR_BLACK);
	LcdDrawInt(278, 26, inSprite->curFrame, LCD_COLOR_WHITE, LCD_COLOR_BLACK);

	// Display palette colors
	for (i = 0; i < inSprite->numColors; i++) {
		LcdDrawRectangle(245, 62 + 12*i, 10, 10, inSprite->palette[i]);
		LcdDrawInt(260, 62 + 12*i, i+1, LCD_COLOR_WHITE, LCD_COLOR_BLACK);
//		LcdDrawHex(275, 62 + 12*i, inSprite->palette[i], LCD_COLOR_WHITE, LCD_COLOR_BLACK);
	}

	inSprite->xpos = 120 - inSprite->width/2;
	inSprite->ypos = 120 - inSprite->height/2;

	LcdDrawInt(10, 10, filePointer, LCD_COLOR_WHITE, LCD_COLOR_BLACK);

	// Draw the sprite
	while (!ALL_BUTTONS) {
		LcdDrawInt(278, 26, inSprite->curFrame, LCD_COLOR_WHITE, LCD_COLOR_BLACK);
		drawSprite(inSprite);
		delayms(50);
		inSprite->curFrame++;
		if (inSprite->curFrame == inSprite->numFrames) inSprite->curFrame = 0;
	}
	
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
			offset = offset;	// nop
			break;
	}

	return filePointer;

}
#endif

void drawSprite(sprite *inSprite) {
	uint16_t temp, i;

	// Move file pointer to beginning of sprite frame
	test_fseek(22 + inSprite->curFrame*inSprite->width*inSprite->height/4UL, TEST_SEEK_SET);

	LcdSetPos(inSprite->xpos, inSprite->ypos, inSprite->xpos + inSprite->width-1, inSprite->ypos + inSprite->height-1);
	LcdWriteCmd(MEMORY_WRITE);
	for (i = 0; i < (inSprite->width * inSprite->height)/4; i++) {
		temp = test_get16();	// Get 4-pixels worth of data
		LcdWriteData(inSprite->palette[(temp & 0x000F)-1]);	// Draw a pixel
		LcdWriteData(inSprite->palette[((temp & 0x00F0)>>4)-1]);	// Draw a pixel
		LcdWriteData(inSprite->palette[((temp & 0x0F00)>>8)-1]);	// Draw a pixel
		LcdWriteData(inSprite->palette[((temp & 0xF000)>>12)-1]);	// Draw a pixel
	}

}

void destroySprite(sprite *inSprite) {
	free(inSprite->palette);
	free(inSprite);
	
	/*******************/
	/* CLOSE FILE HERE */
	/*******************/
}
