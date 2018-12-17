#include "waveplayer.h"

/*!
 * @name Private functions provided with STM32072B Demo
 *
 * These functions were not written by Sparkbox employees, and are included
 * to properly read information from .WAV file headers and convert data from
 * signed to unsigned
 *
 * @{
 */
static void WavePlayer_ReadAndParse(WAV_Format* WAVE_Format);
static void ToggleBufferSign(uint32_t* pBuffer, uint32_t BufferSize);
static uint32_t ReadUnit(uint8_t *buffer, uint8_t idx, uint8_t NbrOfBytes, Endianness BytesFormat);
/* @} */

// Number of bits per DMA transfer
uint8_t transferSize = 16;
// Buffers for playing from SD card
uint16_t *audioBuffer;

// Handles for initialization
DAC_HandleTypeDef hdac;
DMA_HandleTypeDef hdma_dac1;
TIM_HandleTypeDef htim6;

// Globally keep track of the number of plays
volatile int32_t numberPlays;
// Store pointer to the currently playing WAV file struct
WAV_Format *playingWav;
// One global variable to make successive file reads faster
FIL F;

/*!
 * @brief Initializes wave player and allocates memory for audio buffer
 */
void WAV_Init(void)
{
	DAC_ChannelConfTypeDef sConfig;
	TIM_MasterConfigTypeDef sMasterConfig;
	GPIO_InitTypeDef GPIO_InitStruct;

	audioBuffer = (uint16_t*)malloc(sizeof(uint8_t) * AUD_BUF_BYTES);

	// If memory allocation failed, stop here and return
	if (audioBuffer == NULL) return;

	// Clock enable to PORTA and DMA1
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_DMA1_CLK_ENABLE();


	// DMA Initialization
	// Enable the DMA IRQ
	NVIC_SetPriority(DMA1_Stream5_IRQn, 0x40);
	NVIC_EnableIRQ(DMA1_Stream5_IRQn);

	// Initialize DAC channel 1
	hdac.Instance = DAC;
	HAL_DAC_Init(&hdac);
	// Configure DAC trigger for DMA request
	sConfig.DAC_Trigger = DAC_TRIGGER_T6_TRGO;
	sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
	HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1);

	// Timer 6 Configuration
	htim6.Instance = TIM6;
	htim6.Init.Prescaler = 0;
	htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
	// Arbitrary period, will change with WAV file
	htim6.Init.Period = 4353;
	HAL_TIM_Base_Init(&htim6);
	// Configure Timer 6 update event to trigger DAC DMA request
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig);

	// Audio standby pin configuration
	GPIO_InitStruct.Pin = GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	// Set to output high to turn off amplifier
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);
}


/*!
 * @brief Import a .WAV file from a FatFs file system
 *
 * This function reads a .WAV file header into WAVE_Format struct
 * to which W points. With no errors and a size of less than 25 kB,
 * the full audio data is read into memory
 *
 * @param FileName Full path to the specified .WAV file
 * @param W Pointer to corresponding WAVE_Format struct
 *
 * @return Error code specified by the ErrorCode enum
 */
uint8_t WAV_Import(const char* FileName, WAV_Format* W)
{
	UINT BytesRead;
	FRESULT res;

	// Check for Null struct
	if (FileName == NULL || W == NULL) return -1;

	// Make sure video does not read from SD
	frameUpdateOff();
	
	delayms(50);

	// Copy Filename to WAV_Format struct
	strcpy(W->Filename, FileName);

	/* Read the Speech wave file status */
	WavePlayer_ReadAndParse(W);
	if (W->Error != Valid_WAVE_File) {
		return W->Error;
	}

	// Open file and error check
	res = f_open(&F, W->Filename, FA_READ);
	if (res != FR_OK) {
		W->Error = Bad_FileRead;
		return W->Error;
	}

	// Set file pointer to correct position
	f_lseek(&F, W->SpeechDataOffset);

	// Read WAV data and error check
	res = f_read(&F, audioBuffer, (uint32_t)(W->DataSize), &BytesRead);
	if (res != FR_OK) {
		W->Error = Bad_FileRead;
		return W->Error;
	}

	// Close file
	f_close(&F);

	// Video can now read from SD
	frameUpdateOn();

	// Convert 16 bit signed to 16 bit unsigned
	ToggleBufferSign((uint32_t*)audioBuffer, W->DataSize / 4);

	return 0;
}

/*!
 * @brief  DMA transfer complete
 */
void DMA1_Stream5_IRQHandler(void)
{
	// Max DMA transfer size 65535 samples > 25

	HAL_DMA_IRQHandler(&hdma_dac1);

	// Update the correct number of plays left
	if (numberPlays == 1) {
		// Done playing,
		WAV_Pause();
		numberPlays = 0;
	} else if (numberPlays <= REPEAT_ALWAYS) {
		numberPlays = REPEAT_ALWAYS;
	} else {
		numberPlays--;
	}

}

/*!
 * @brief Play a .WAV file that has been successfully imported
 *
 * This function plays a .WAV file imported with WAV_Import(). If numPlays is
 * 0, nothing will happen. If numPlays is negative, the .WAV file will repeat
 * until WAV_Pause() or WAV_Destroy() are called.
 *
 * @param W Pointer to a WAVE_Format previously imported
 * @param numPlays Number of times to repeat the .WAV file
 *
 */
void WAV_Play(WAV_Format* W, int numPlays)
{
	// Pause old WAV file
	WAV_Pause();

	// Do not play if number of plays is 0 or error
	if (numPlays == 0 || W == NULL || W->Error != Valid_WAVE_File) return;

	// If numPlays is negative, set it to REPEAT_ALWAYS (-1)
	// This allows interrupt to call this function with a decrement
	if (numPlays < 0) numPlays = REPEAT_ALWAYS;

	// Save the currently playing WAV file
	playingWav = W;

	/* Save data for DMA interrupt */
	numberPlays = numPlays;

	/* Save the transfer size of the current WAV file */
	transferSize = playingWav->BitsPerSample;

	/* Deinitialize everything */
	HAL_DAC_Stop(&hdac, DAC_CHANNEL_1);
	HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1);
	HAL_DAC_DeInit(&hdac);

	// Set period based on sample rate
	HAL_TIM_Base_DeInit(&htim6); // deinit
	htim6.Init.Period = W->TIM6ARRValue; // set ARR value
	HAL_TIM_Base_Init(&htim6); // init

	// Initialize DAC with correct transfer size
	HAL_DAC_Init(&hdac);

	// Start TIM6 and turn on amplifier
	WAV_Resume();

	// Start DAC
	HAL_DAC_Start(&hdac, DAC_CHANNEL_1);

	// Start DAC with DMA (12 Bit DAC)
	// 12 bit left alignment ignores 4 lsb of 16 bit data
	HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*)audioBuffer,
		playingWav->DataSize / 2, DAC_ADDR);
}

/*!
 * @brief Pauses the currently playing .WAV file and turns off the audio amp
 */
void WAV_Pause(void)
{
	/* Disable TIM6 */
	HAL_TIM_Base_Stop(&htim6);
	/* Set to output high to turn off amplifier */
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);
}

/*!
 * @brief Resumes playing the imported .WAV file and turns on the audio amp
 */
void WAV_Resume(void)
{
	/* Enable TIM6 */
	HAL_TIM_Base_Start(&htim6);
	/* Set to output low to turn on amplifier */
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);
}

/*!
 * @brief Stops playing the .WAV file and deinitializes the .WAV peripherals
 */
void WAV_Destroy(void)
{
	// Pause the currently playing WAV file
	WAV_Pause();

	// Deinitialize Timer 6
	HAL_TIM_Base_DeInit(&htim6);

	// Deinitialize DAC
	HAL_DAC_Stop(&hdac, DAC_CHANNEL_1);
	HAL_DAC_DeInit(&hdac);

	// Stop DMA 1 Stream 5
	HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1);

	// Disable DMA interrupt
	NVIC_DisableIRQ(DMA1_Stream5_IRQn);

	// Free previously allocated audio buffer
	free(audioBuffer);
}


/************************************************************************/
/** ALL FUNCTIONS BELOW ARE FULLY COPIED OR SLGIHTLY CHANGED FROM CODE **/
/** PROVIDED BY ST FOR STM32072B-EVAL DEMONSTRATION. NO SPARKBOX ********/
/** EMPLOYEES ARE CLAIMING CREDIT FOR ANY CODE BELOW THIS POINT *********/
/************************************************************************/


/*
 * Attempts to read and parse a WAV file on SD card
 * Any error code is stored in WAVE_Format->Error
 */
void WavePlayer_ReadAndParse(WAV_Format* WAVE_Format)
{
	UINT BytesRead;
	uint32_t temp = 0x00;
	uint32_t extraformatbytes = 0;
	uint16_t TempBuffer[_MAX_SS];
	uint8_t res;

	res = f_open(&F, WAVE_Format->Filename, FA_READ);
	if (res) {
		WAVE_Format->Error = Bad_FileRead;
		return;
	}


	res = f_read(&F, TempBuffer, _MAX_SS, &BytesRead);
	if (res) {
		WAVE_Format->Error = Bad_FileRead;
		return;
	}

	/* Read chunkID, must be 'RIFF'  -------------------------------------------*/
	temp = ReadUnit((uint8_t*)TempBuffer, 0, 4, BigEndian);
	if(temp != CHUNK_ID){
		f_close(&F);
		WAVE_Format->Error = Bad_RIFF_ID;
		return;
	}

	/* Read the file length ----------------------------------------------------*/
	WAVE_Format->RIFFchunksize = ReadUnit((uint8_t*)TempBuffer, 4, 4, LittleEndian);

	/* Read the file format, must be 'WAVE' ------------------------------------*/
	temp = ReadUnit((uint8_t*)TempBuffer, 8, 4, BigEndian);
	if(temp != FILE_FORMAT){
		f_close(&F);
		WAVE_Format->Error = Bad_WAVE_Format;
		return;
	}

	/* Read the format chunk, must be'fmt ' ------------------------------------*/
	temp = ReadUnit((uint8_t*)TempBuffer, 12, 4, BigEndian);
	if(temp != FORMAT_ID){
		f_close(&F);
		WAVE_Format->Error = Bad_FormatChunk_ID;
		return;
	}
	/* Read the length of the 'fmt' data, must be 0x10 -------------------------*/
	temp = ReadUnit((uint8_t*)TempBuffer, 16, 4, LittleEndian);
	if(temp != 0x10){
		extraformatbytes = 1;
	}
	/* Read the audio format, must be 0x01 (PCM) -------------------------------*/
	WAVE_Format->FormatTag = ReadUnit((uint8_t*)TempBuffer, 20, 2, LittleEndian);
	if(WAVE_Format->FormatTag != WAVE_FORMAT_PCM){
		f_close(&F);
		WAVE_Format->Error = Bad_FormatTag;
		return;
	}

	/* Read the number of channels, must be 0x01 (Mono) ----------------------*/
	WAVE_Format->NumChannels = ReadUnit((uint8_t*)TempBuffer, 22, 2, LittleEndian);

	if(WAVE_Format->NumChannels != CHANNEL_MONO){
		f_close(&F);
		WAVE_Format->Error = Bad_Number_Of_Channel;
		return;
	}

	/* Read the Sample Rate ----------------------------------------------------*/
	WAVE_Format->SampleRate = ReadUnit((uint8_t*)TempBuffer, 24, 4, LittleEndian);
	/* Update the OCA value according to the .WAV file Sample Rate */
	// Only allow up to 50 ksps rate to ensure buffer has enough time to update
	if (WAVE_Format->SampleRate < SAMPLE_RATE_MIN ||
		WAVE_Format->SampleRate > SAMPLE_RATE_MAX) {
			f_close(&F);
			WAVE_Format->Error = Bad_Sample_Rate;
			return;
	}

	/* Fs = Ftimer / (ARR+1); ARR = Ftimer / Fs - 1 */
	WAVE_Format->TIM6ARRValue = (uint32_t)(TIM6FREQ / WAVE_Format->SampleRate - 1);

	/* Read the Byte Rate ------------------------------------------------------*/
	WAVE_Format->ByteRate = ReadUnit((uint8_t*)TempBuffer, 28, 4, LittleEndian);

	/* Read the block alignment ------------------------------------------------*/
	WAVE_Format->BlockAlign = ReadUnit((uint8_t*)TempBuffer, 32, 2, LittleEndian);

	/* Read the number of bits per sample --------------------------------------*/
	WAVE_Format->BitsPerSample = ReadUnit((uint8_t*)TempBuffer, 34, 2, LittleEndian);

	if (WAVE_Format->BitsPerSample != BITS_PER_SAMPLE_16 &&
		WAVE_Format->BitsPerSample != BITS_PER_SAMPLE_8) {
		f_close(&F);
		WAVE_Format->Error = Bad_Bits_Per_Sample;
		return;
	}
	WAVE_Format->SpeechDataOffset = 36;
	/* If there is Extra format bytes, these bytes will be defined in "Fact Chunk" */
	if(extraformatbytes == 1){
		/* Read th Extra format bytes, must be 0x00 ------------------------------*/
		temp = ReadUnit((uint8_t*)TempBuffer, 36, 2, LittleEndian);
		if(temp != 0x00){
			f_close(&F);
			WAVE_Format->Error = Bad_ExtraFormatBytes;
                        return;
		}
		/* Read the Fact chunk, must be 'fact' -----------------------------------*/
		temp = ReadUnit((uint8_t*)TempBuffer, 38, 4, BigEndian);
		if(temp != FACT_ID){
			f_close(&F);
			WAVE_Format->Error = Bad_FactChunk_ID;
                        return;
		}
		/* Read Fact chunk data Size ---------------------------------------------*/
		temp = ReadUnit((uint8_t*)TempBuffer, 42, 4, LittleEndian);
		WAVE_Format->SpeechDataOffset += 10 + temp;
	}
	/* Read the Data chunk, must be 'data' -------------------------------------*/
	temp = ReadUnit((uint8_t*)TempBuffer, WAVE_Format->SpeechDataOffset, 4, BigEndian);
	WAVE_Format->SpeechDataOffset += 4;
	if(temp != DATA_ID){
		f_close(&F);
		WAVE_Format->Error = Bad_DataChunk_ID;
		return;
	}

	/* Read the number of sample data ------------------------------------------*/
	temp = ReadUnit((uint8_t*)TempBuffer, WAVE_Format->SpeechDataOffset, 4, LittleEndian);

	if (temp > 50000 || temp < 0) {
		f_close(&F);
		WAVE_Format->Error = Bad_DataSize;
		return;
	}
	WAVE_Format->DataSize = temp;

	WAVE_Format->SpeechDataOffset += 4;
	f_close(&F);
	WAVE_Format->Error = Valid_WAVE_File;
	return;
}

/*!
 * @brief  Toggles sign bit of input buffer.
 * @param  pBuffer: pointer to the input buffer
 * @param  BufferSize: the size of the buffer in words
 * @retval None
 */
static void ToggleBufferSign(uint32_t* pBuffer, uint32_t BufferSize)
{
	volatile uint32_t readdata = (uint32_t)pBuffer;
	volatile uint32_t loopcnt = 0;

	/* Invert sign bit: PCM format is 16-bit signed and DAC is 12-bit unsigned */
	for(loopcnt = 0; loopcnt < BufferSize; loopcnt++){
		*(uint32_t*)readdata ^= 0x80008000;
		readdata+=4;
	}
}

/*!
 * @brief  Reads a number of bytes from the SPI Flash and reorder them in Big
 *         or little endian.
 * @param  NbrOfBytes: number of bytes to read.
 *         This parameter must be a number between 1 and 4.
 * @param  ReadAddr: external memory address to read from.
 * @param  Endians: specifies the bytes endianness.
 *         This parameter can be one of the following values:
 *             - LittleEndian
 *             - BigEndian
 * @retval Bytes read from the SPI Flash.
 */
static uint32_t ReadUnit(uint8_t *buffer, uint8_t idx, uint8_t NbrOfBytes, Endianness BytesFormat)
{
	uint32_t index = 0;
	uint32_t temp = 0;

	for (index = 0; index < NbrOfBytes; index++){
		temp |= buffer[idx + index] << (index * 8);
	}

	if(BytesFormat == BigEndian){
		temp = __REV(temp);
	}
	return temp;
}
