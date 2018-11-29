#include "video.h"

uint16_t *videoBuffer1;
uint16_t *videoBuffer2;

uint16_t *indexBuffer;

// 0 if currently playing buffer 2 (reading 1)
// 1 if currently playing buffer 1 (reading 2)
uint8_t videoBuffer = 0;

uint8_t bufferTransfers = NUM_TRANSFERS;
// Define statements to select READ_BUFFER or PLAY_BUFFER
// based on which is being played and which is being filled
#define READ_BUFFER (videoBuffer ? videoBuffer2 : videoBuffer1)
#define PLAY_BUFFER (videoBuffer ? videoBuffer1 : videoBuffer2)

DMA_HandleTypeDef hdma_tim7_up;
TIM_HandleTypeDef htim7;
FIL BUF;


void toggleVideoBuffers(void);
FRESULT readToVideoBuffer(void);

void initVideo(void)
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
		indexBuffer == NULL) return;

	// Initialize DMA for video
	/* DMA controller clock enable */
	__HAL_RCC_DMA1_CLK_ENABLE();

	/* DMA interrupt init */
	/* DMA1_Stream2_IRQn interrupt configuration */
	NVIC_SetPriority(DMA1_Stream2_IRQn, 0x39);
	NVIC_EnableIRQ(DMA1_Stream2_IRQn);

	// Initialize Timer 7 for DMA
	htim7.Instance = TIM7;
	htim7.Init.Prescaler = 0;
	htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim7.Init.Period = TIM7PSC;
	HAL_TIM_Base_Init(&htim7);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig);
	
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
	HAL_DMA_Abort_IT(&hdma_tim7_up);

	// Initialize LCD to be ready for continuous data
	LcdSetPos(0, 0, LCD_WIDTH, LCD_HEIGHT);
    LcdWriteCmd(MEMORY_WRITE);
	
	// read new frame into one videoBuffer
	readToVideoBuffer();
	toggleVideoBuffers();

	// Frame update is beginning, set FPS pin high
	LCD_FPS_HIGH;

	// Start new transfer, configuring DMA user callbacks
	HAL_DMA_Start_IT(&hdma_tim7_up, (uint32_t)PLAY_BUFFER, (uint32_t)(fsmc_data),
	(uint32_t)VID_BUF_BYTES / 2);
	
	// Enable timer 7 to start DMA
	HAL_TIM_Base_Start(&htim7);
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
void DMA1_Stream2_IRQHandler(void)
{
	HAL_DMA_IRQHandler(&hdma_tim7_up);

	// Stop transfer if it is somehow still occurring
	HAL_DMA_Abort_IT(&hdma_tim7_up);

	// Update number of buffer transfers remaining
	bufferTransfers--;
	toggleVideoBuffers();

	if (bufferTransfers == 0) {
		// Frame is completely written to LCD, get ready for next transfer
		// Reset buffer transfer counter
		bufferTransfers = NUM_TRANSFERS;
		
		// Stop timer to stop DMA
		HAL_TIM_Base_Stop(&htim7);

		// Done updating frame, set FPS pin low
		LCD_FPS_LOW;
	} else {
		// Start new transfer, configuring DMA user callbacks
		HAL_DMA_Start_IT(&hdma_tim7_up, (uint32_t)PLAY_BUFFER, (uint32_t)(fsmc_data), 
		(uint32_t)VID_BUF_BYTES / 2);
	
		// Read new data into other buffer
		readToVideoBuffer();
	}

	// Because we have the time, read new audio data as well
	WAV_Update();
}
