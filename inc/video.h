/*!
 * @file video.h
 * @author Mason Roach
 * @author Patrick Roy
 * @date Dec 13 2018
 * 
 * @brief This file contains functions to control Sparkbox video buffering
 *
 * These functions control the video buffering on the sparkbox LCD.
 *
 */
#ifndef SPARKBOX_VIDEO
#define SPARKBOX_VIDEO

#include "stm32f4xx.h"
#include "ff.h"
#include "lcd.h"
#include "sprite.h"
#include "waveplayer.h"
#include "led.h"

/*!
 * @brief Size of LCD in bytes, not pixels
 */
#define LCD_SIZE_BYTES (LCD_PIXELS * 2)

/*!
 * @brief Number of rows per transfer
 *
 * @note Use a multiple of 2
 */
#define LCD_TRANSFER_ROWS 8

/*!
 * @brief Number of buffer transfers to fill LCD
 */
#define NUM_TRANSFERS (LCD_HEIGHT / LCD_TRANSFER_ROWS)

/*!
 * @brief Throw compile error for an invalid number of row transfers
 */
#if (LCD_SIZE_BYTES % NUM_TRANSFERS)
#error "LCD_TRANSFER_ROWS must evenly divide LCD_HEIGHT."
#endif

/*!
 * @brief Video buffer size in bytes
 *
 * @note For LCD_TRANSFER_ROWS = 8, this is 5120 Bytes
 */
#define VID_BUF_BYTES (LCD_SIZE_BYTES / NUM_TRANSFERS)

/*!
 * @brief Default background color
 */
#define VIDEO_BG COLOR_888_TO_565(0x00A591)

/*!
 * @brief Calculate timer values based on number of rows being transferred
 * at a time
 */
#define TIM7PSC 99
#define TIM7ARR (75 * LCD_TRANSFER_ROWS - 1)

/*!
 * @brief Frames per second at which the video updates
 *
 * @note If the video cannot process a new frame in time,
 * that frame will be "skipped" and not updated
 */
#define FPS 20

/*!
 * @brief Prescaler and ARR value of the frame update timer determined
 * by the framerate
 */
#define TIM10PSC 4199
#define TIM10ARR (40000 / FPS - 1)


/*!
 * @brief Variable containing address to write pixel data
 */
extern volatile uint16_t * const fsmc_data;

/*!
 * @brief Layers of sprite structs to access sprite pixel data
 */
extern spriteList layers;

/*!
 * @brief Initializes video peripherals
 *
 * This function allocates required memory and initializes video peripherals.
 * These peripherals include Timers 7 and 10 as well as DMA transfers. 
 *
 * @note This function allocates memory for both video buffers and a buffer
 * for reading sprite pixels from the FatFs file system
 *
 * @return 0 on success, -1 on failure
 */
int8_t initVideo(void);

/*!
 * @brief Updates a frame using sprite data and reading from FatFs file system
 *
 * This function is used to update the frame on screen using two buffers.
 * While one buffer is filling with new pixel data, the other is sent with DMA
 * to the LCD. Timer 7 is used to generate interrupts that will read new data if
 * neccessary.
 *
 * @note This function will set up interrupts to trigger that will periodically
 * read from the FatFs file system. The user should not be accessing the FatFs
 * file system during the time the frame is updating.
 */
void updateFrame(void);

/*!
 * @brief Turn on automatically updating frames
 *
 * Calling this function will activate automatically updating frames 
 * at the frame rate specified by the FPS define statement
 *
 */
void frameUpdateOn(void);

/*!
 * @brief Turn off automatically updating frames
 *
 * Calling this function will deactivate automatically updating frames.
 * To update a frame, the user must now call updateFrame() for each new frame
 *
 */
void frameUpdateOff(void);

#endif
