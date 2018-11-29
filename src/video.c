#include "video.h"

volatile uint16_t *videoBuffer1;
volatile uint16_t *videoBuffer2;

volatile uint16_t *indexBuffer;

volatile uint16_t color = LCD_COLOR_RED;

// 0 if currently playing buffer 2 (reading 1)
// 1 if currently playing buffer 1 (reading 2)
volatile uint8_t videoBuffer = 0;

volatile uint8_t bufferTransfers = 0;
volatile uint8_t txComplete = 0;
// Define statements to select READ_BUFFER or PLAY_BUFFER
// based on which is being played and which is being filled
#define READ_BUFFER (videoBuffer ? videoBuffer2 : videoBuffer1)
#define PLAY_BUFFER (videoBuffer ? videoBuffer1 : videoBuffer2)

DMA_HandleTypeDef hdma_memtomem_dma2_stream5;
TIM_HandleTypeDef htim7;
FIL BUF;


void toggleVideoBuffers(void);
FRESULT readToVideoBuffer(void);

int8_t initVideo(void)
{
	TIM_MasterConfigTypeDef sMasterConfig;

	// Allocate memory for video buffers
	videoBuffer1 = (uint16_t*)malloc(sizeof(uint8_t) * VID_BUF_BYTES);
	videoBuffer2 = (uint16_t*)malloc(sizeof(uint8_t) * VID_BUF_BYTES);
	// Buffer storing indexes of palette data is 1/4 size of video buffers
	indexBuffer = (uint16_t*)malloc(sizeof(uint8_t) * VID_BUF_BYTES / 4);

	// If memory allocation failed, stop here and return
	if (videoBuffer1 == NULL || 
		videoBuffer2 == NULL || 
		indexBuffer == NULL) return -1;

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
/**
	* @brief Reads the corrct data into the correct video buffer
	*
	*
	*/
FRESULT readToVideoBuffer(void)
{
	// Based on number of transfers left, read correct data into READ_BUFFER
	// bufferTransfers stores number of unfinished transfers to LCD with DMA

	// Read indexes into indexBuffer from SD

	// Write actual colors to READ_BUFFER

	uint16_t i;

	// Fill buffer with color
	for (i=0; i<(VID_BUF_BYTES / 2); i++) {
		*(READ_BUFFER + i) = color;
	}

	if (color == LCD_COLOR_RED) color = LCD_COLOR_GREEN;
	else if (color == LCD_COLOR_GREEN) color = LCD_COLOR_BLUE;
	else if (color == LCD_COLOR_BLUE) color = LCD_COLOR_BLACK;
	else color = LCD_COLOR_RED;

	return FR_OK;
}

/**
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
	
	// read new frame into one videoBuffer
	readToVideoBuffer();
	toggleVideoBuffers();
	readToVideoBuffer();
	
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
    bufferTransfers++;

	
}

/*
 * Toggles which buffer is being filled and which is being played
 */
void toggleVideoBuffers(void)
{
    videoBuffer ^= 0x01;
}

/**
	* @brief DMA transfer complete
	*
	*
	*/
void DMA2_Stream5_IRQHandler(void)
{
	HAL_DMA_IRQHandler(&hdma_memtomem_dma2_stream5);
	
	txComplete = 1;
}

// Time for another DMA transfer
void TIM7_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim7);
	
	// If DMA has not completed, do not start anything new
	
    // Stop transfer if it is somehow still occurring
    // HAL_DMA_Abort_IT(&hdma_memtomem_dma2_stream5);

    // Update number of buffer transfers remaining
    toggleVideoBuffers();

	// Start the new transfer before reading
	HAL_DMA_Start_IT(&hdma_memtomem_dma2_stream5,
	(uint32_t)PLAY_BUFFER, (uint32_t)(fsmc_data),
	(uint32_t)(VID_BUF_BYTES / 2));

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
    }

    bufferTransfers++;
    // Because we have the time, read new audio data as well
    // WAV_Update();
}
