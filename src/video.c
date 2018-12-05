#include "video.h"

volatile uint16_t *videoBuffer1;
volatile uint16_t *videoBuffer2;

// 0 if currently playing buffer 2 (reading 1)
// 1 if currently playing buffer 1 (reading 2)
volatile uint8_t videoBuffer = 0;

volatile uint8_t bufferTransfers = 0;
// Define statements to select READ_BUFFER or PLAY_BUFFER
// based on which is being played and which is being filled
#define READ_BUFFER (videoBuffer ? videoBuffer2 : videoBuffer1)
#define PLAY_BUFFER (videoBuffer ? videoBuffer1 : videoBuffer2)

DMA_HandleTypeDef hdma_memtomem_dma2_stream5;
TIM_HandleTypeDef htim7;
FIL BUF;

void toggleVideoBuffers(void);
FRESULT readToVideoBuffer(void);

// Initialize timers, DMA, and allocate memory for video buffers
int8_t initVideo(void)
{
	TIM_MasterConfigTypeDef sMasterConfig;

	// Allocate memory for video buffers
	videoBuffer1 = (uint16_t*)malloc(sizeof(uint8_t) * VID_BUF_BYTES);
	videoBuffer2 = (uint16_t*)malloc(sizeof(uint8_t) * VID_BUF_BYTES);

	// If memory allocation failed, stop here and return
	if (videoBuffer1 == NULL
		|| videoBuffer2 == NULL
//		|| indexBuffer == NULL
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
	
	return 0;
}
/*
 * @brief Reads the corrct data into the correct video buffer
 *
 *
 */
FRESULT readToVideoBuffer(void)
{
	uint8_t set;

	// Based on number of transfers left, read correct data into READ_BUFFER
	// bufferTransfers stores number of unfinished transfers to LCD with DMA

	// Read indexes into indexBuffer from SD

	// Write actual colors to READ_BUFFER

	for (set = 0; set < LCD_TRANSFER_ROWS/4; set++) getNext4Rows(set);

	return FR_OK;
}

/*
 * @brief Update the frame on the LCD
 *
 *
 */
void updateFrame(void)
{
	// Stop old DMA transfers
	HAL_DMA_Abort_IT(&hdma_memtomem_dma2_stream5);

	// Reset completed number of transfers
	bufferTransfers = 0;
	
	// Reset file pointers of sprites
	updateSprites();
	seekStartOfFrames();

	// read new frame into one videoBuffer
	readToVideoBuffer();
	toggleVideoBuffers();
	
	// Frame update is beginning, set FPS pin high
	LCD_FPS_HIGH;

	// Enable timer 7 after DMA start
	HAL_TIM_Base_Start_IT(&htim7);
	// Initialize LCD to be ready for continuous data
	LcdSetPos(0, 0, LCD_WIDTH, LCD_HEIGHT);
    LcdWriteCmd(MEMORY_WRITE);
	
	// Start new transfer, configuring DMA user callbacks
	HAL_DMA_Start_IT(&hdma_memtomem_dma2_stream5, 
	(uint32_t)PLAY_BUFFER, (uint32_t)(fsmc_data),
	(uint32_t)(VID_BUF_BYTES / 2));

	// Increment number of transfers that have started
    bufferTransfers++;

	readToVideoBuffer();

}

/*
 * Toggles which buffer is being filled and which is being played
 */
void toggleVideoBuffers(void)
{
    videoBuffer = !videoBuffer;
}

/**
	* @brief DMA transfer complete
	*
	*
	*/
void DMA2_Stream5_IRQHandler(void)
{
	HAL_DMA_IRQHandler(&hdma_memtomem_dma2_stream5);
}

// Time for another DMA transfer
void TIM7_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim7);

	toggleVideoBuffers();
	
	// Start the new transfer before reading
	HAL_DMA_Start_IT(&hdma_memtomem_dma2_stream5,
	(uint32_t)PLAY_BUFFER, (uint32_t)(fsmc_data),
	(uint32_t)(VID_BUF_BYTES / 2));

	// Increment number of transfers that have started
    bufferTransfers++;

	// Determine if we need to read more data or if we are done
    if (bufferTransfers < NUM_TRANSFERS) {
        // Read new data into other buffer
        readToVideoBuffer();
    } else {
        // Frame is completely written to LCD, get ready for next transfer
        // Reset buffer transfer counter
        bufferTransfers = 0;

		// Stop timer to not trigger this interrupt again
		HAL_TIM_Base_Stop_IT(&htim7);

        // Done updating frame, set FPS pin low
        LCD_FPS_LOW;
		
    	// Done with video, need to update audio
    	WAV_Update();
    }

}

// For testing purposes of the videoGetRow function
uint8_t testGetRow(const uint8_t row) {
	uint8_t layer;
	uint16_t pixel;
	static uint16_t temp;
	static uint8_t index = 0;

	// TODO: index values and temp values will have to be stored for each sprite
	
	// Iterate through finding the value fo each pixel in the row
	for (pixel = 0; pixel < LCD_WIDTH; pixel++) {

		// Check each layer for a valid pixel
		for (layer = 0; layer < spriteLayers.size; layer++) {

			// Check bounds of the sprite
			if ((row >= spriteLayers.sprites[layer]->ypos) &&
			 (row < spriteLayers.sprites[layer]->ypos + spriteLayers.sprites[layer]->height) &&
			 (pixel >= spriteLayers.sprites[layer]->xpos) &&
			 (pixel < spriteLayers.sprites[layer]->xpos + spriteLayers.sprites[layer]->width)) {

				// Valid bounds, fetch the pixel of the sprite
				if (index == 0) {
					temp = test_get16();	// Fetch 4 pixels of data
					index = 3;	// Reset the index value
				} else {
					// Shift the index before checking the pixel
					index--;
				}

				// TODO: this value should be stored if it does not equal 0.
				//       Should inline assembly be used here?
				// Check the alpha value of the pixel
				if (temp & (0xF000 >> (index * 4))) {

					// TODO: Put the pixel into the video buffer
					
					// For testing purposes, just place the pixel
					LcdPutPixel(pixel, row, spriteLayers.sprites[layer]->palette
						[((temp & (0xF000 >> (index * 4))) >> ((3-index) * 4))-1]);
					
					// When a pixel is found, stop the nail, ignore other layers
					goto pixelFound;
				}

				// If the pixel is transparent, move to the next layer
			
			}
		
		}

		// If a non-transparent pixel was not found on all layers,
		// use the default background color
		// TODO: Put the pixel into the video buffer
		
		// For testing purposes, just place the pixel
		LcdPutPixel(pixel, row, VIDEO_BG);	
pixelFound:
		pixel = pixel;		// nop();
	}

	return 0;
}

// For testing purposes of the videoGetRow function
uint8_t getNext4Rows(uint8_t set) {
	uint8_t row;
	uint8_t layer;
	uint16_t pixel;
	uint16_t fetched[MAX_LAYERS];
	uint8_t index[MAX_LAYERS] = {0};

	// TODO: index values and temp values will have to be stored for each sprite

	// Do 4 rows at a time
	for (row = 0; row < 4; row++) {
	
		// Iterate through finding the value fo each pixel in the row
		for (pixel = 0; pixel < LCD_WIDTH; pixel++) {

			// Check each layer for a valid pixel
			for (layer = 0; layer < spriteLayers.size; layer++) {

				// Check bounds of the sprite
				if ((row >= spriteLayers.sprites[layer]->ypos) &&
				 (row < spriteLayers.sprites[layer]->ypos + spriteLayers.sprites[layer]->height) &&
				 (pixel >= spriteLayers.sprites[layer]->xpos) &&
				 (pixel < spriteLayers.sprites[layer]->xpos + spriteLayers.sprites[layer]->width)) {

					// Valid bounds, fetch the pixel of the sprite
					if (index[layer] == 0) {
						fetched[layer] = test_get16();	// Fetch 4 pixels of data
						index[layer] = 3;	// Reset the index value
					} else {
						// Shift the index before checking the pixel
						index[layer]--;
					}

					// TODO: this value should be stored if it does not equal 0.
					//       Should inline assembly be used here?
					// Check the alpha value of the pixel
					if (fetched[layer] & (0xF000 >> (index[layer] * 4))) {

						// TODO: Put the pixel into the video buffer
						READ_BUFFER[LCD_WIDTH*row + pixel + set*FOUR_ROW_OFFSET] = spriteLayers.sprites[layer]->palette[((fetched[layer] & (0xF000 >> (index[layer] * 4))) >> ((3-index[layer]) * 4))-1];
						// For testing purposes, just place the pixel
//						LcdPutPixel(pixel, row, spriteLayers.sprites[layer]->palette
//							[((fetched[layer] & (0xF000 >> (index[layer] * 4))) >> ((3-index[layer]) * 4))-1]);
						
						// When a pixel is found, stop the nail, ignore other layers
						goto pixelFound;
					}

					// If the pixel is transparent, move to the next layer
				
				}
			
			}

			// If a non-transparent pixel was not found on all layers,
			// use the default background color
			// TODO: Put the pixel into the video buffer
			READ_BUFFER[LCD_WIDTH*row + pixel + set*FOUR_ROW_OFFSET] = VIDEO_BG;
			
			// For testing purposes, just place the pixel
			//LcdPutPixel(pixel, row, VIDEO_BG);	
	pixelFound:
			pixel = pixel;		// nop();
		}

	}

	return 0;

}
