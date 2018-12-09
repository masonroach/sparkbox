#include "waveplayer.h"
#include "lcd.h"

static void WavePlayer_ReadAndParse(WAV_Format* WAVE_Format);
static void ToggleBufferSign(uint32_t* pBuffer, uint32_t BufferSize);
static uint32_t ReadUnit(uint8_t *buffer, uint8_t idx, uint8_t NbrOfBytes, Endianness BytesFormat);

// Buffers for playing from SD card
uint16_t *audioBuffer1;
uint16_t *audioBuffer2;

// 0 if currently playing buffer 2 (reading 1)
// 1 if currently playing buffer 1 (reading 2)
uint8_t buffer = 0;

// 0 if neither buffer needs to be read before the other finishes
// 1 if one buffer needs to be read before the other finishes playing
uint8_t readyToRead = 0;

// Define statements to select READ_BUFFER or PLAY_BUFFER
// based on which is being played and which is being filled
#define READ_BUFFER (buffer ? audioBuffer2 : audioBuffer1)
#define PLAY_BUFFER (buffer ? audioBuffer1 : audioBuffer2)

// Handles for initialization
DAC_HandleTypeDef hdac;
DMA_HandleTypeDef hdma_dac1;
TIM_HandleTypeDef htim6;

// Globally keep track of the number of plays
int32_t numberPlays;
WAV_Format *playingWav;
FIL F;


/**
  * @brief  Toggles which buffer is being filled and which is being played
  * @param  None
  * @retval None
 */
void toggleBuffers(void)
{
	buffer = !buffer;
}

/**
  * @brief  Wave player Initialization
  * @param  None
  * @retval None
  */
void WAV_Init(void)
{
	DAC_ChannelConfTypeDef sConfig;
	TIM_MasterConfigTypeDef sMasterConfig;
	GPIO_InitTypeDef GPIO_InitStruct;
	
	audioBuffer1 = (uint16_t*)malloc(sizeof(uint8_t) * AUD_BUF_BYTES);
	audioBuffer2 = (uint16_t*)malloc(sizeof(uint8_t) * AUD_BUF_BYTES);

	// If memory allocation failed, stop here and return
	if (audioBuffer1 == NULL || audioBuffer2 == NULL) return;

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
	// Arbitrary 
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


/**
	* @brief  Reads .WAV file and saves needed info in struct W
	* @param  FileName: pointer to the wave file name to be read
	* @param  FileLen: pointer to the wave struct
	* @retval None
	*/
uint8_t WAV_Import(const char* FileName, WAV_Format* W)
{
	// Copy Filename to WAV_Format struct
	strcpy(W->Filename, FileName);
 
	/* Read the Speech wave file status */
	WavePlayer_ReadAndParse(W);

	if (W->Error != Valid_WAVE_File) {
		return W->Error;
	}
	

	return 0;
}  
	
/**
	* @brief  DMA transfer complete, begin next transfer
	* @param  None
	* @retval None
	*/
void DMA1_Stream5_IRQHandler(void)
{
	// When this function is called, it is assumed READ_BUFFER
	// is correctly filled. Therefore, playingWav->DataPos will be "ahead"
	// of the transfer that just completed by AUD_BUF_BYTES

	// PLay entire buffer unless changed later
	uint32_t bufferSize = AUD_BUF_BYTES / 2;
	
	HAL_DMA_IRQHandler(&hdma_dac1);

	// Determine if a WAV file has completed
	// If it has, the next sample we read will be at positon
	// less than the size of the audio buffer
	if (playingWav->DataPos < AUD_BUF_BYTES) {
		// Determine if playing the file should end
    	if (numberPlays == 1) {
			// Done, stop timer and amplifier but set up 
			// next transfer from start of file
    	    WAV_Pause();
			numberPlays = 0;
			// Reset next data read to position 0
			playingWav->DataPos = 0;
    	} else if (numberPlays <= REPEAT_ALWAYS) {
    	    numberPlays = REPEAT_ALWAYS;
    	} else {
    	    numberPlays--;
    	}

		// For a WAV restart, other buffer is ready so play it
		// Timer is stopped for non repeating WAV files
	}
	
	// Adjust the buffer size to less than full buffer if there is 
	// only 1 play remaining and remaining samples won't fill the buffer
	if (numberPlays == 1 && (playingWav->DataPos < AUD_BUF_BYTES)) {
		// Not repeating another time and audio buffer is larger than
		// remaining number of samples to be played
		// Only play remaining samples, even though buffer is filled
		// as if the file is repeating
		bufferSize = (AUD_BUF_BYTES - playingWav->DataPos) / 2;
	}

	// Swap which buffer is playing before playing the newly filled buffer
	toggleBuffers();

	// Always begin a new transfer, timer will be off if playing is over
	HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*)PLAY_BUFFER,
	bufferSize, DAC_ALIGN_12B_L);
	
	// Set read ready flag so the other buffer can be filled
	// while the current transfer completes
	readyToRead = 1;
}

/*
 * This function updates the WAV file buffers
 * by reading in new samples to fill the buffer
 * If not enough samples exist before end of file,
 * this will fill the remainder from the beginning
 * of the file. This means a WAV file cannot be less than
 * half of the size of the buffer (Over 1600 2 byte samples)
 * This function must be called again with enough time to 
 * execute before the other buffer is done playing
 */
FRESULT WAV_Update(void)
{
	FRESULT res;
	UINT BytesRead;
	
	// If WAV data does not need to be read, return
	if (!readyToRead) return 0;

	LCD_FPS_HIGH;

	/* Open wave data file */
    res = f_open(&F, playingWav->Filename, FA_READ);
	if (res != FR_OK) return res;
	
    /* Jump to correct wave data */
    res = f_lseek(&F, playingWav->SpeechDataOffset + playingWav->DataPos);
	if (res != FR_OK) return res;

	// Read WAV data
	if (playingWav->DataPos + AUD_BUF_BYTES > playingWav->DataSize) {
		// If this case executes, there must be two reads to fill 
		// the audio buffer completely

		// This first read will reach the end of the file
        res = f_read(&F, READ_BUFFER, 
		playingWav->DataSize - playingWav->DataPos, &BytesRead);
		if (res != FR_OK) return res;

        // Set file pointer to beginning of data again
        res = f_lseek(&F, playingWav->SpeechDataOffset);
		if (res != FR_OK) return res;

        // This second read will fill the remainder of the buffer
        res = f_read(&F, (uint16_t*)((uint8_t*)READ_BUFFER + playingWav->DataSize),
        playingWav->DataPos + AUD_BUF_BYTES - playingWav->DataSize, &BytesRead);
		if (res != FR_OK) return res;

		// Update position for next read
		playingWav->DataPos = (playingWav->DataPos + AUD_BUF_BYTES)- playingWav->DataSize;

	} else {
		// There must only be one read to fill audio buffer completely
        res = f_read(&F, READ_BUFFER, AUD_BUF_BYTES, &BytesRead);
		if (res != FR_OK) return res;

		// Update position for next read
        playingWav->DataPos += AUD_BUF_BYTES;

		// Handle the case where number of remaining file samples equals
		// number of audio buffer samples
		if (playingWav->DataPos == playingWav->DataSize) {
			playingWav->DataPos = 0;
		}
	}

    /* Close file */
    f_close(&F);

	/* Convert 16 Bit Signed to 16 Bit Unsigned */
	ToggleBufferSign((uint32_t*)READ_BUFFER, AUD_BUF_BYTES / 4);

	/* Finished reading into the read buffer, no longer ready to read */
	readyToRead = 0;
	
	LCD_FPS_LOW;
	
	return res;
}

/**
	* @brief  Starts playing audio stream from the audio Media. Only
	* works for audio files of size not less than half of 
	* the audio buffer size (6400 bytes)
	* 
	* @retval None
	*/
void WAV_Play(WAV_Format* W, int numPlays)
{
	uint32_t bufferSize;
	
	// Do not play if number of plays is 0
	if (numPlays == 0) return;

	// If numPlays is negative, set it to REPEAT_ALWAYS (-1)
	// This allows interrupt to call this function with a decrement
	if (numPlays < 0) numPlays = REPEAT_ALWAYS;
	
	// Reset data position to 0
	W->DataPos = 0;

	// Save the currently playing WAV file
	playingWav = W;
	
	/* Save data for DMA interrupt */
	numberPlays = numPlays;

	/* Set read ready flag */
	readyToRead = 1;
	
	/* Read correct data into read buffer */
	WAV_Update();
	
	/* Deinitialize everything */
	WAV_Pause();
    HAL_DAC_Stop(&hdac, DAC_CHANNEL_1);
    HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1);

	// Set period based on sample rate
	HAL_TIM_Base_DeInit(&htim6); // deinit
	htim6.Init.Period = W->TIM6ARRValue; // set ARR value
	HAL_TIM_Base_Init(&htim6); // init
	
	// Start TIM6 and turn on amplifier
	WAV_Resume();

	// Start DAC
	HAL_DAC_Start(&hdac, DAC_CHANNEL_1);

	// If the buffer size is greater than the WAV file
	// and no repeats, only play WAV file contents
	if (W->DataPos < AUD_BUF_BYTES && numberPlays == 1) {
		bufferSize = W->DataSize / 2;
	} else {
		bufferSize = AUD_BUF_BYTES;
	}
	
	// Swap read and play buffers so we play the data just read
	toggleBuffers();
	
	// Start DAC with DMA (12 Bit DAC)
	// 12 bit left alignment ignores 4 lsb of 16 bit data
	HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*)PLAY_BUFFER,
		bufferSize, DAC_ALIGN_12B_L);

	// Set flag so the other buffer can be filled now
	readyToRead = 1;
	

}

void WAV_Pause(void)
{
	/* Disable TIM6 */
	HAL_TIM_Base_Stop(&htim6);
	/* Set to output high to turn off amplifier */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);
}

/*
 * WAV_Resume is designed so after a file has finished playing a fixed number
 * of times, a call to this function will cause the file to play forever.
 */
void WAV_Resume(void)
{
	/* Enable TIM6 */
	HAL_TIM_Base_Start(&htim6);
	/* Set to output low to turn on amplifier */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);
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
		WAVE_Format->Error = FileReadFailed;
		return;
	}

	res = f_read(&F, TempBuffer, _MAX_SS, &BytesRead);
	if (res) {
		WAVE_Format->Error = FileReadFailed;
		return;
	}

	/* Read chunkID, must be 'RIFF'  -------------------------------------------*/
	temp = ReadUnit((uint8_t*)TempBuffer, 0, 4, BigEndian);
	if(temp != CHUNK_ID){
		f_close(&F);
		WAVE_Format->Error = Unvalid_RIFF_ID;
		return;
	}

	/* Read the file length ----------------------------------------------------*/
	WAVE_Format->RIFFchunksize = ReadUnit((uint8_t*)TempBuffer, 4, 4, LittleEndian);

	/* Read the file format, must be 'WAVE' ------------------------------------*/
	temp = ReadUnit((uint8_t*)TempBuffer, 8, 4, BigEndian);
	if(temp != FILE_FORMAT){
		f_close(&F);
		WAVE_Format->Error = Unvalid_WAVE_Format;
		return;
	}

	/* Read the format chunk, must be'fmt ' ------------------------------------*/
	temp = ReadUnit((uint8_t*)TempBuffer, 12, 4, BigEndian);
	if(temp != FORMAT_ID){
		f_close(&F);
		WAVE_Format->Error = Unvalid_FormatChunk_ID;
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
		WAVE_Format->Error = Unsupporetd_FormatTag;
		return;
	}

	/* Read the number of channels, must be 0x01 (Mono) ----------------------*/
	WAVE_Format->NumChannels = ReadUnit((uint8_t*)TempBuffer, 22, 2, LittleEndian);
	
	if(WAVE_Format->NumChannels != CHANNEL_MONO){
		f_close(&F);
		WAVE_Format->Error = Unsupporetd_Number_Of_Channel;
		return;
	}

	/* Read the Sample Rate ----------------------------------------------------*/
	WAVE_Format->SampleRate = ReadUnit((uint8_t*)TempBuffer, 24, 4, LittleEndian);
	/* Update the OCA value according to the .WAV file Sample Rate */
	// Only allow up to 50 ksps rate to ensure buffer has enough time to update
	if (WAVE_Format->SampleRate < 6000 || WAVE_Format->SampleRate > 50000) {
			f_close(&F);
			WAVE_Format->Error = Unsupporetd_Sample_Rate;
			return;
	}
	
	/* Fs = Ftimer / (ARR+1); ARR = Ftimer / Fs - 1 */
	WAVE_Format->TIM6ARRValue = (uint32_t)((float)(TIM6FREQ) / (float)(WAVE_Format->SampleRate) - 1.0);
	
	/* Read the Byte Rate ------------------------------------------------------*/
	WAVE_Format->ByteRate = ReadUnit((uint8_t*)TempBuffer, 28, 4, LittleEndian);

	/* Read the block alignment ------------------------------------------------*/
	WAVE_Format->BlockAlign = ReadUnit((uint8_t*)TempBuffer, 32, 2, LittleEndian);

	/* Read the number of bits per sample --------------------------------------*/
	WAVE_Format->BitsPerSample = ReadUnit((uint8_t*)TempBuffer, 34, 2, LittleEndian);
	
	if (WAVE_Format->BitsPerSample != BITS_PER_SAMPLE_16) {
		f_close(&F);
		WAVE_Format->Error = Unsupporetd_Bits_Per_Sample;
		return;
	}
	WAVE_Format->SpeechDataOffset = 36;
	/* If there is Extra format bytes, these bytes will be defined in "Fact Chunk" */
	if(extraformatbytes == 1){
		/* Read th Extra format bytes, must be 0x00 ------------------------------*/
		temp = ReadUnit((uint8_t*)TempBuffer, 36, 2, LittleEndian);
		if(temp != 0x00){
			f_close(&F);
			WAVE_Format->Error = Unsupporetd_ExtraFormatBytes;
                        return;
		}
		/* Read the Fact chunk, must be 'fact' -----------------------------------*/
		temp = ReadUnit((uint8_t*)TempBuffer, 38, 4, BigEndian);
		if(temp != FACT_ID){
			f_close(&F);
			WAVE_Format->Error = Unvalid_FactChunk_ID;
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
		WAVE_Format->Error = Unvalid_DataChunk_ID;
		return;
	}

	/* Read the number of sample data ------------------------------------------*/
	WAVE_Format->DataSize = 
	ReadUnit((uint8_t*)TempBuffer, WAVE_Format->SpeechDataOffset, 4, LittleEndian);

	WAVE_Format->SpeechDataOffset += 4;
	f_close(&F);
	WAVE_Format->Error = Valid_WAVE_File;
	return;
}

/**
	* @brief  Toggles sign bit of input buffer.
	* @param  pBuffer: pointer to the input buffer  
	* @param  BufferSize: the size of the buffer in words
	* @retval None
	*/
static void ToggleBufferSign(uint32_t* pBuffer, uint32_t BufferSize)
{
	uint32_t readdata = (uint32_t)pBuffer;
	uint32_t loopcnt = 0;
	
	/* Invert sign bit: PCM format is 16-bit signed and DAC is 12-bit unsigned */
	for(loopcnt = 0; loopcnt < BufferSize; loopcnt++){
		*(uint32_t*)readdata ^= 0x80008000;
		readdata+=4;
	}
}

/**
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
