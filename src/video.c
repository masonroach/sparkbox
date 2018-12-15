#include "video.h"

// Static function prototypes
static uint8_t getNextRows(void);
static void toggleVideoBuffers(void);
static FRESULT readToVideoBuffer(void);

uint16_t *videoBuffer1;
uint16_t *videoBuffer2;

// 0 if currently playing buffer 2 (reading 1)
// 1 if currently playing buffer 1 (reading 2)
uint8_t videoBuffer = 0;

uint8_t bufferTransfers = 0;
// Define statements to select READ_BUFFER or PLAY_BUFFER
// based on which is being played and which is being filled
#define READ_BUFFER (videoBuffer ? videoBuffer2 : videoBuffer1)
#define PLAY_BUFFER (videoBuffer ? videoBuffer1 : videoBuffer2)

uint8_t transferComplete = 0;
uint8_t frameComplete = 0;
uint8_t readComplete = 0;

DMA_HandleTypeDef hdma_memtomem_dma2_stream5;
TIM_HandleTypeDef htim7;
FIL BUF;

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

	transferComplete = 1;
	frameComplete = 1;
	readComplete = 1;
	
	return 0;
}
/*
 * @brief Reads the corrct data into the correct video buffer
 *
 *
 */
static FRESULT readToVideoBuffer(void)
{
	// Based on number of transfers left, read correct data into READ_BUFFER
	// bufferTransfers stores number of unfinished transfers to LCD with DMA

	// Read indexes into indexBuffer from SD

	// Write actual colors to READ_BUFFER
	
	readComplete = 0;

	getNextRows();
	
	readComplete = 1;

	return FR_OK;
}

/*
 * @brief Update the frame on the LCD
 *
 *
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

	readToVideoBuffer();

}

/*
 * Toggles which buffer is being filled and which is being played
 */
static void toggleVideoBuffers(void)
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
	// Use HAL library to handle lower level interrupt
	HAL_DMA_IRQHandler(&hdma_memtomem_dma2_stream5);

	// Signal that the transfer is complete
	transferComplete = 1;
}

// Time for another DMA transfer
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
    if (bufferTransfers < NUM_TRANSFERS) {
        // Read new data into other buffer
        readToVideoBuffer();
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

static uint8_t getNextRows(void) {
	uint8_t row;
	uint8_t l;
	uint16_t pixel;
	uint8_t lcdRow;
	uint8_t paletteIndex;
	static uint32_t fetched[MAX_LAYERS];
	static uint8_t index[MAX_LAYERS] = {0};

	if (bufferTransfers == 0) {
		for (l = 0; l < MAX_LAYERS; l++) {
			fetched[l] = 0;
			index[l] = 0;
		}
	}

	// Do 2 rows at a time
	for (row = 0; row < LCD_TRANSFER_ROWS; row++) {
	
		lcdRow = bufferTransfers*LCD_TRANSFER_ROWS + row;
	
		// Iterate through finding the value fo each pixel in the row
		for (pixel = 0; pixel < LCD_WIDTH; pixel++) {

			// Initial value
			READ_BUFFER[pixel + LCD_WIDTH*row] = VIDEO_BG;

			// Check each layer for a valid pixel
			for (l = 0; l < layers.size; l++) {

				// Check bounds of the sprite
				if ((lcdRow >= layers.spr[l]->ypos) &&
				 (lcdRow < layers.spr[l]->ypos + layers.spr[l]->height) &&
				 (pixel >= layers.spr[l]->xpos) &&
				 (pixel < layers.spr[l]->xpos + layers.spr[l]->width)) {

					// Valid bounds, fetch the pixel of the sprite
					if (index[l] == 0) {
						if (f_read(&layers.spr[l]->file, &fetched[l], 4, NULL)) {	// Fetch 8 pixels of data
							ledError(LED_ERROR);
							while(1);
						}
						index[l] = 7;	// Reset the index value
					} else {
						// Shift the index before checking the pixel
						index[l]--;
					}

					paletteIndex = (fetched[l] >> ((7-index[l])*4)) & 0x0F;

					// Check the alpha value of the pixel
					if (paletteIndex) {

						// Pixel is valid, get the color
						READ_BUFFER[pixel + LCD_WIDTH*row] = layers.spr[l]->palette[paletteIndex - 1];
						break;
					}

				}
			
			}

			// For the rest of the pixels, increment index but don't check color
			for (l++ ; l < layers.size; l++) {
				// Check bounds of the sprite
				if ((lcdRow >= layers.spr[l]->ypos) &&
				 (lcdRow < layers.spr[l]->ypos + layers.spr[l]->height) &&
				 (pixel >= layers.spr[l]->xpos) &&
				 (pixel < layers.spr[l]->xpos + layers.spr[l]->width)) {

					// Valid bounds, fetch the pixel of the sprite
					if (index[l] == 0) {
						if (f_read(&layers.spr[l]->file, &fetched[l], 4, NULL)) {	// Fetch 8 pixels of data
							ledError(LED_ERROR);
							while(1);
						}
						index[l] = 7;	// Reset the index value
					} else {
						// Shift the index before checking the pixel
						index[l]--;
					}		

				}
	
			}

		}

	}

	return 0;

}
