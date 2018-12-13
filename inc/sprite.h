/*!
 * @file sprite.h
 * @author Mason Roach
 * @author Patrick Roy
 * @date Dec 13 2018
 *
 * @brief Functions to interact with sprites
 *
 * These functions are the basic functions that should be used to interact with
 * sprites.
 *
 * Layout of a Sparkbox .spr file
 * | Name      | Size       | Description                                      |
 * |:----------|:----------:|:------------------------------------------------:|
 * | Width     |  16-bits   | Width of the sprite                              |
 * | Height    |  16-bits   | Height of the sprite                             |
 * | numColors |   4-bits   | Number of colors in the palette                  |
 * | Reserved  |   4-bits   | Reserved                                         |
 * | numFrames |   8-bits   | Number of frames in the sprite                   |
 * |Reserved[4]|16-bits each| Reserved                                         |
 * |Palette[15]|16-bits each| RGB565 colors in the palette                     |
 * |ColorMap[x]|4-bits each | Map of each pixel to the color in the palette    |
 *
 * @todo Include code examples
 */

#ifndef SPARK_SPRITE
#define SPARK_SPRITE

#include <stdlib.h>
#include <stdint.h>
#include "ff.h"
#include "lcd.h"
#include "button.h"
#include "led.h"

/*!
 * @brief Limits the number of layers available for sprites
 */
#define MAX_LAYERS 8

/*!
 * @brief Limits the number of sprites that can be allocated at once
 */
#define MAX_SPRITES 32


/*!
 * @brief The sprite struct itself
 */
typedef struct {
	FIL file;	/*!< File struct used by FatFS */ 
	uint16_t width;	/*!< Width of the sprite */
	uint16_t height;	/*!< Height of the sprite */
	int16_t xpos;	/*!< x position of the sprite */
	int16_t ypos;	/*!< y position of the sprite */
	int16_t xvelocity;	/*!< x velocity of the sprite */
	int16_t yvelocity;	/*!< y velocity of the sprite */
	uint16_t numColors;	/*!< Number of colors in the palette */
	uint16_t *palette;	/*!< Array of RGB565 colors */
	uint8_t numFrames;	/*!< Total number of frames in the sprite sheet */
	uint8_t curFrame;	/*!< Current frame index */
	uint8_t flags;	/*!< Flags of the sprite */
	uint8_t tag;	/*!< Index of the sprite in the spritesAllocated list */
	int8_t layer;	/*!< Current layer of the sprite */
} sprite;

/*!
 * @brief Used to keep track of arrays
 */
typedef struct {
	sprite **spr;	/*!< Array of pointers to sprites stored */
	uint8_t size;	/*!< Number of sprites stored */
} spriteList;

/*!
 * @brief Various flags available for the sprites
 */
typedef enum {
	HIDE = 0x80,	/*!< If set, will not draw the sprite on the LCD */
	ANIMATED = 0x40,	/*!< If set, the sprite will cycle through its frames */
//	reserved = 0x20,
//	reserved = 0x10,
//	reserved = 0x08,
//	reserved = 0x04,
//	reserved = 0x02,
//	reserved = 0x01
} SPRITE_FLAG;

/*!
 * @brief Return values of various sprite functions
 */
typedef enum {
	NOT_ENOUGH_MEMORY = 1, /*!< Not enough memory to allocate more space */
	NO_FILE_ACCESS = 2, /*!< Cannot access the file on the SD card */
	TOO_MANY_SPRITES = 3, /*!< Too many sprites either allocated or on layers */
	FILE_ERROR = 4, /*!< Misc file error */
} SPRITE_ERROR;

// sprite functions
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
int8_t initSprite(sprite *targetSprite, char *filename);

/*!
 * @brief Copy one sprite to another
 *
 * @param inSprite Sprite to copy data from
 * @param targetSprite Sprite to copy data to
 *
 * @warning This function has not been written yet
 */
int8_t copySprite(sprite *inSprite, sprite *targetSprite);

/*!
 * @brief Frees memory allocated by a sprite
 * 
 * Destroys the sprite by freeing memory of the palette and closing the file on
 * the SD card. The sprite is also removed from both spritesAllocated and
 * spriteLayers list.
 * 
 * @param inSprite Sprite struct to destroy
 */
void destroySprite(sprite *inSprite);

/*!
 * @brief Draw the full sprite on the screen
 * @note This does not work the same way as video, this should only be used for
 * debugging purposes.
 * 
 * @param inSprite Sprite to draw
 * @return Number of pixels drawn
 */
uint32_t drawSprite(sprite *inSprite);

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
void updateSprites(void);

/*!
 * @brief Set the xpos value of the given sprite
 *
 * @param inSprite Pointer to the sprite struct to change
 * @param x New x value
 */
void spriteSetXpos(sprite *inSprite, int16_t x);

/*!
 * @brief Set the ypos value of the given sprite
 *
 * @param inSprite Pointer to the sprite struct to change
 * @param y New y value
 */
void spriteSetYpos(sprite *inSprite, int16_t y);

/*!
 * @brief Set the xpos and ypos value of the given sprite
 *
 * @param inSprite Pointer to the sprite struct to change
 * @param x New x value
 * @param y New y value
 */
void spriteSetPos(sprite *inSprite, int16_t x, int16_t y);

/*!
 * @brief Set the flag bits of the given sprite
 *
 * | Bit    | Name   | Description                                             |
 * |--------|:------:|:-------------------------------------------------------:|
 * |X0000000| Hide   | If set to 1, will not draw the sprite on the LCD        |
 * |0X000000|Animated| if set to 1, the sprite will cycle through frames      
 * |00X00000|Reserved| |
 * |000X0000|Reserved| |
 * |0000X000|Reserved| |
 * |00000X00|Reserved| |
 * |000000X0|Reserved| |
 * |0000000X|Reserved| |
 *
 * @param inSprite Pointer to the sprite struct to change
 * @param flagVals New flag values
 */
void spriteSetFlags(sprite *inSprite, uint8_t flagVals);

/*!
 * @brief Set or clear the hide flag of the given sprite
 * 
 * @param inSprite Pointer to the sprite struct to change
 * @param hideEnable single bit to either set or clear the HIDE flag
 */
void spriteHide(sprite *inSprite, uint8_t hideEnable);

/*!
 * @brief Set or clear the animate flag of the given sprite
 *
 * @param inSprite Pointer to the sprite struct to change
 * @param animationEnable single bit to either set or clear the ANIMATED flag
 */
void spriteAnimate(sprite *inSprite, uint8_t animationEnable);

/*!
 * @brief Set a palette color of a sprite to a new given color
 *
 * @param inSprite Pointer to the sprite struct to change
 * @param index Index of the palette color to change, 0 <= index <= 14 
 * @param color New color to place in the palette (RGB565 format)
 */
void spriteSetPaletteColor(sprite *inSprite, uint8_t index, uint16_t color);

// spriteLayers functions
/*!
 * @brief Add a sprite pointer to the spriteLayer list at the given position
 *
 * @param inSprite Pointer to the sprite to add to the list
 * @param layer Layer number to add the sprite to
 *
 * @return 0 on success, !0 on failure
 */
uint8_t spriteLayersInsert(sprite *inSprite, uint8_t layer);

/*!
 * @brief Append a sprite pointer to the spriteLayer list
 *
 * Adds the sprite to the end of the list, which will be the top-most layer
 * 
 * @param inSprite Pointer to the sprite to add
 *
 * @return 0 on success, !0 on failure
 */
uint8_t spriteLayersAdd(sprite *inSprite);

/*!
 * @brief Remove the given sprite from the spriteLayers list
 *
 * @param inSprite Pointer to sprite to remove from the list
 *
 * @return 0 on success, !0 on failure
 */
uint8_t spriteLayersRemove(sprite *inSprite);

/*!
 * @brief For every sprite in the spriteLayers, move their file pointers to
 * beginning of their current frame
 *
 * @return 0 on success, !0 on failure
 */
uint8_t seekStartOfFrames(void);

// test functions
void drawSpriteDebug(sprite *inSprite);

#endif
