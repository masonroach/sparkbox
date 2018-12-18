/*!
 * @file video.c
 * @author Mason Roach
 * @author Patrick Roy
 * @date Dec 13 2018
 * 
 * @brief This file contains functions to control Sparkbox video buffering
 *
 * These functions control the video buffering on the sparkbox LCD.
 *
 */
#include "video.h"

// Static function prototypes
/*!
 * @brief Fills a video buffer with data to be sent to the LCD
 *
 * @note This function reads from the FatFs file system. On a read fail,
 * the error LED on the Sparkbox turns on and this program hangs in a 
 * dead loop
 *
 * @return 0 on success
 */
static uint8_t getNextRows(void);

/*!
 * @brief Toggles the video buffers currently playing and reading
 */
static void toggleVideoBuffers(void);

/*!
 * @brief Fills a video buffer using the getNextRows function and
 * controls flags for read complete 
 *
 * @return 0 on success
 */
static FRESULT readToVideoBuffer(void);

// Video buffers containing data for the LCD
uint16_t *videoBuffer1;
uint16_t *videoBuffer2;

// 2D array storing sprite pallete indexes read from FatFs 
uint32_t **fetched;


// 0 if currently playing buffer 2 (reading 1)
// 1 if currently playing buffer 1 (reading 2)
uint8_t videoBuffer = 0;

uint8_t bufferTransfers = 0;
// Define statements to select READ_BUFFER or PLAY_BUFFER
// based on which is being played and which is being filled
#define READ_BUFFER (videoBuffer ? videoBuffer2 : videoBuffer1)
#define PLAY_BUFFER (videoBuffer ? videoBuffer1 : videoBuffer2)

// Flags used to prevent data writing and reading errors
uint8_t frameUpdate = 0;
uint8_t transferComplete = 0;
uint8_t frameComplete = 0;
uint8_t readComplete = 0;

// Handles for initialization
DMA_HandleTypeDef hdma_memtomem_dma2_stream5;
TIM_HandleTypeDef htim7;
TIM_HandleTypeDef htim10;

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
int8_t initVideo(void)
{
	TIM_MasterConfigTypeDef sMasterConfig;
	uint8_t i;

	// Allocate memory for video buffers
	videoBuffer1 = (uint16_t*)malloc(sizeof(uint8_t) * VID_BUF_BYTES);
	videoBuffer2 = (uint16_t*)malloc(sizeof(uint8_t) * VID_BUF_BYTES);

	// Allocate first dimension of 2D array
	fetched = (uint32_t**)malloc(sizeof(uint32_t*) * MAX_LAYERS);

	// Allocate second dimension of 2D array
	for (i=0; i<MAX_LAYERS; i++) {
		fetched[i] = (uint32_t*)malloc(sizeof(uint8_t) * (LCD_WIDTH / 2));
		if (fetched[i] == NULL) return -1;
	}

	// If memory allocation failed, stop here and return
	if (videoBuffer1 == NULL
		|| videoBuffer2 == NULL
		|| fetched == NULL
	) return -1;

	// Initialize DMA for video
	/* DMA controller clock enable */
	__HAL_RCC_DMA2_CLK_ENABLE();

	/* Configure DMA request hdma_memtomem_dma2_stream5 on DMA2_Stream5 */
	hdma_memtomem_dma2_stream5.Instance = DMA2_Stream5;
	hdma_memtomem_dma2_stream5.Init.Channel = DMA_CHANNEL_0;
	hdma_memtomem_dma2_stream5.Init.Direction = DMA_MEMORY_TO_MEMORY;
	hdma_memtomem_dma2_stream5.Init.PeriphInc = DMA_PINC_ENABLE;
	hdma_memtomem_dma2_stream5.Init.MemInc = DMA_MINC_DISABLE;
	hdma_memtomem_dma2_stream5.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
	hdma_memtomem_dma2_stream5.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
	hdma_memtomem_dma2_stream5.Init.Mode = DMA_NORMAL;
	hdma_memtomem_dma2_stream5.Init.Priority = DMA_PRIORITY_VERY_HIGH;
	hdma_memtomem_dma2_stream5.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
	hdma_memtomem_dma2_stream5.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_1QUARTERFULL;
	hdma_memtomem_dma2_stream5.Init.MemBurst = DMA_MBURST_SINGLE;
	hdma_memtomem_dma2_stream5.Init.PeriphBurst = DMA_PBURST_SINGLE;
	HAL_DMA_Init(&hdma_memtomem_dma2_stream5);

	/* DMA interrupt init */
	/* DMA2_Stream5_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA2_Stream5_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA2_Stream5_IRQn);

	// Initialize Timer 7 for DMA
	htim7.Instance = TIM7;
	htim7.Init.Prescaler = TIM7PSC;
	htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim7.Init.Period = TIM7ARR;
	HAL_TIM_Base_Init(&htim7);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_ENABLE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig);

	// Initialize TIM 10 to trigger at 20 Hz
	htim10.Instance = TIM10;
	htim10.Init.Prescaler = TIM10PSC;
	htim10.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim10.Init.Period = TIM10ARR;
	htim10.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	HAL_TIM_Base_Init(&htim10);
	
	// Turn off frame updating
	frameUpdateOff();

	// Start timer 10
	HAL_TIM_Base_Start_IT(&htim10);

	transferComplete = 1;
	frameComplete = 1;
	readComplete = 1;
	
	return 0;
}


/*!
 * @brief This function handles TIM1 update interrupt and TIM10 interrupt
 */
void TIM1_UP_TIM10_IRQHandler(void)
{

	HAL_TIM_IRQHandler(&htim10);

	// Update the frame if it is on
	if (frameUpdate) updateFrame();
}

/*!
 * @brief Turn on automatically updating frames
 *
 * Calling this function will activate automatically updating frames 
 * at the frame rate specified by the FPS define statement
 *
 */
void frameUpdateOn(void)
{
	frameUpdate = 1;
}

/*!
 * @brief Turn off automatically updating frames
 *
 * Calling this function will deactivate automatically updating frames.
 * To update a frame, the user must now call updateFrame() for each new frame
 *
 */
void frameUpdateOff(void)
{
	frameUpdate = 0;
}

/*!
 * @brief Fills a video buffer using the getNextRows function and
 * controls flags for read complete 
 *
 * @return 0 on success
 */
static FRESULT readToVideoBuffer(void)
{
	// Signal read not complete
	readComplete = 0;

	// Read
	getNextRows();
	
	// Signal read complete
	readComplete = 1;

	return FR_OK;
}

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
void updateFrame(void)
{
	// Do not update new frame until old is completely written
	if (!frameComplete) return;

	// Frame update is beginning, set FPS pin high
	LCD_FPS_HIGH;

	// Stop old DMA transfers
	HAL_DMA_Abort_IT(&hdma_memtomem_dma2_stream5);

	// Reset completed number of transfers
	bufferTransfers = 0;
	
	// Reset file pointers of sprites
	updateSprites();
	seekStartOfFrames();

	// read new frame into one videoBuffer
	readToVideoBuffer();

	// Swap read and play buffers
	toggleVideoBuffers();
	
	// Enable timer 7
	HAL_TIM_Base_Start_IT(&htim7);
	// Initialize LCD to be ready for continuous data
	LcdSetPos(0, 0, LCD_WIDTH, LCD_HEIGHT);
    LcdWriteCmd(MEMORY_WRITE);
	
	// Start new transfer, configuring DMA user callbacks
	HAL_DMA_Start_IT(&hdma_memtomem_dma2_stream5, 
	(uint32_t)PLAY_BUFFER, (uint32_t)(fsmc_data),
	(uint32_t)(VID_BUF_BYTES / 2));

	// Indicate current transfer is not complete
	transferComplete = 0;

	// Increment number of transfers that have started
    bufferTransfers++;

	// Read to the other video buffer
	readToVideoBuffer();

}

/*!
 * @brief Toggles the video buffers currently playing and reading
 */
static void toggleVideoBuffers(void)
{
    videoBuffer = !videoBuffer;
}

/*!
 * @brief DMA transfer complete
 */
void DMA2_Stream5_IRQHandler(void)
{
	// Use HAL library to handle lower level interrupt
	HAL_DMA_IRQHandler(&hdma_memtomem_dma2_stream5);

	// Signal that the transfer is complete
	transferComplete = 1;
}

/*!
 * @brief Timer 7 interrupt handler
 */
void TIM7_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim7);

	// If the previous transfer is not done, do not do anything
	if (!transferComplete || !readComplete) return;

	// Reset transferComplete flag
	transferComplete = 0;

	// Toggle read and play buffers
	toggleVideoBuffers();
	
	// Start the new transfer before reading
	HAL_DMA_Start_IT(&hdma_memtomem_dma2_stream5,
	(uint32_t)PLAY_BUFFER, (uint32_t)(fsmc_data),
	(uint32_t)(VID_BUF_BYTES / 2));

	// Increment number of transfers that have started
    bufferTransfers++;

	// Determine if we need to read more data or if we are done
    if (bufferTransfers <= NUM_TRANSFERS) {
		if (bufferTransfers != NUM_TRANSFERS) {
        	// Read new data into other buffer
        	readToVideoBuffer();
		}
    } else {
        // Frame is completely written to LCD, get ready for next transfer
        // Reset buffer transfer counter
        bufferTransfers = 0;

		// Stop timer to not trigger this interrupt again
		HAL_TIM_Base_Stop_IT(&htim7);

		// Indicate frame is complete
		frameComplete = 1;

        // Done updating frame, set FPS pin low
        LCD_FPS_LOW;
    }

}

/*!
 * @brief Fills a video buffer with data to be sent to the LCD
 *
 * @note This function reads from the FatFs file system. On a read fail,
 * the error LED on the Sparkbox turns on and this program hangs in a 
 * dead loop
 *
 * @return 0 on success
 */
static uint8_t getNextRows(void) {
	uint8_t row;
	uint8_t l;
	uint16_t pixel;
	uint8_t lcdRow;
	uint8_t paletteIndex;

	// Do 2 rows at a time
	for (row = 0; row < LCD_TRANSFER_ROWS; row++) {
	
		lcdRow = bufferTransfers*LCD_TRANSFER_ROWS + row;

		// Read a row of pixels for all sprites with a row on this row
		for (l = 0; l<layers.size; l++) {		
			if ((lcdRow >= layers.spr[l]->ypos) &&
		 	 (lcdRow < layers.spr[l]->ypos + layers.spr[l]->height)) {
				if (f_read(&layers.spr[l]->file, fetched[l], 
				           layers.spr[l]->width / 2, NULL)) {
					ledError(LED_ERROR);
					while(1);
				}
			}
		}

		// Iterate through finding the value for each pixel in the row
		for (pixel = 0; pixel < LCD_WIDTH; pixel++) {

			// Check each layer for a valid pixel
			for (l = 0; l < layers.size; l++) {

				// Check bounds of the sprite
				if ((lcdRow >= layers.spr[l]->ypos) &&
				 (lcdRow < layers.spr[l]->ypos + layers.spr[l]->height) &&
				 (pixel >= layers.spr[l]->xpos) &&
				 (pixel < layers.spr[l]->xpos + layers.spr[l]->width)) {

					// Find pallette index from previously read values
					paletteIndex = (fetched[l][(pixel-(layers.spr[l]->xpos))/8] >> 
					               (((pixel-layers.spr[l]->xpos) % 8)*4)) & 0x000F;

					// Check the alpha value of the pixel
					if (paletteIndex) {

						// Pixel is valid, get the color
						READ_BUFFER[pixel+LCD_WIDTH*row] = 	
						  layers.spr[l]->palette[paletteIndex-1];

						// Found a non transparent pixel, skip to next pixel
						break;
					} else {
						// Default background color
						READ_BUFFER[pixel + LCD_WIDTH*row] = VIDEO_BG;
					}

				} else {
					// Default background color
					READ_BUFFER[pixel + LCD_WIDTH*row] = VIDEO_BG;
				}
				
			
			}

		}

	}

	return 0;

}
