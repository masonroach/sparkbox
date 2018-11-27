/**
  ******************************************************************************
  * @file    waveplayer.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    16-January-2014
  * @brief   This file includes the Wave Player driver for the STM32072B-EVAL 
  *          demonstration.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "waveplayer.h"

/** @addtogroup STM32072B_EVAL_Demo
  * @{
  */

/** @defgroup WAVEPLAYER_Private_FunctionPrototypes
  * @{
  */
void toggleBuffers(void);
static void WavePlayer_ReadAndParse(const char* WavName, WAV_Format* WAVE_Format);
static void ToggleBufferSign(uint32_t* pBuffer, uint32_t BufferSize);
uint32_t ReadUnit(uint8_t *buffer, uint8_t idx, uint8_t NbrOfBytes, Endianness BytesFormat);
/**
  * @}
  */

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

/** @defgroup WAVEPLAYER_Private_Functions
  * @{
  */

/*
 * Toggles which buffer is being filled and which is being played
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
	
	audioBuffer1 = (uint16_t*)malloc(sizeof(uint8_t) * AUD_BUF_BYTES);
	audioBuffer2 = (uint16_t*)malloc(sizeof(uint8_t) * AUD_BUF_BYTES);

	// If memory allocation failed, stop here and return
	if (audioBuffer1 == NULL || audioBuffer2 == NULL) return;

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/********************************* DMA Config *******************************/
    /* DMA1 clock enable (to be used with DAC) */
    __HAL_RCC_DMA1_CLK_ENABLE();
    /* DMA channel3 Configuration is managed by WAV_Play() function */


    /* Enable the DMA IRQ --------------------------------------*/
    NVIC_SetPriority(DMA1_Stream5_IRQn, 0x40);
    NVIC_EnableIRQ(DMA1_Stream5_IRQn);

	/******************************* Init DAC channel 1 (PA4) *******************/

    hdac.Instance = DAC;
    HAL_DAC_Init(&hdac);

    /** DAC channel OUT1 config **/
    sConfig.DAC_Trigger = DAC_TRIGGER_T6_TRGO;
    sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
    HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1);

    /************************** Init DAC channel 2 (PA5) ************************/

	/******************************* TIM6 Configuration *************************/
	
	htim6.Instance = TIM6;
	htim6.Init.Prescaler = 0;
	htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim6.Init.Period = 4353;
	HAL_TIM_Base_Init(&htim6);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig);

}

/*
 * Attempts to read and parse a WAV file on SD card
 * Any error code is stored in WAVE_Format->Error
 */
void WavePlayer_ReadAndParse(const char* WavName, WAV_Format* WAVE_Format)
{
	UINT BytesRead;
	uint32_t temp = 0x00;
	uint32_t extraformatbytes = 0;
	uint16_t TempBuffer[_MAX_SS];
	uint8_t res;
	
	f_open(&F, WavName, FA_READ);

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
	if (WAVE_Format->SampleRate < 6000 || WAVE_Format->SampleRate > 100000) {
			f_close(&F);
			WAVE_Format->Error = Unsupporetd_Sample_Rate;
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
	WAVE_Format->DataSize = ReadUnit((uint8_t*)TempBuffer, WAVE_Format->SpeechDataOffset, 
										4, LittleEndian);

	WAVE_Format->SpeechDataOffset += 4;
	f_close(&F);
	WAVE_Format->Error = Valid_WAVE_File;
	return;
}

/**
	* @brief  Reads .WAV file and saves needed info in struct W
	* @param  FileName: pointer to the wave file name to be read
	* @param  FileLen: pointer to the wave struct
	* @retval None
	*/
uint8_t WAV_Import(const char* FileName, WAV_Format* W)
{
	uint8_t i = 0;

	/* Read the Speech wave file status */
	WavePlayer_ReadAndParse((const char*)FileName, W);

	// Copy Filename up to 64 characters to WAV_Format struct
	while (FileName[i] != '\0' && i < 64) {
		W->Filename[i] = FileName[i];
	} 

	if (W->Error != Valid_WAVE_File) {
		return W->Error;
	}
	

	return 0;
}  
	
/**
	* @brief  DMA transfer complete
	* @param  None
	* @retval None
	*/
void DMA1_Stream5_IRQHandler(void)
{
	// PLay entire buffer unless changed later
	uint32_t bufferSize = AUD_BUF_BYTES / 2;
	
	// This function is responsible for setting the next read position
	// playingWav->DataPos is location of next sample to read from file
	
	HAL_DMA_IRQHandler(&hdma_dac1);

	// Has a WAV file just completed?
	if (playingWav->DataPos - AUD_BUF_BYTES < 0) {
		// Determine to stop playing WAV file
    	if (numberPlays == 1) {
			// Done, stop timer but set up transfer in case user
			// wishes to restart WAV file
    	    HAL_TIM_Base_Stop(&htim6);
			numberPlays = 0;
			// Reset next data read
			playingWav->DataPos = 0;
    	} else if (numberPlays <= REPEAT_ALWAYS) {
    	    numberPlays = REPEAT_ALWAYS;
    	} else {
    	    numberPlays--;
    	}

		// For a WAV restart, other buffer is ready so play it
		// Timer is stopped for non repeating WAV files
	
		playingWav->DataPos -= playingWav->DataSize;

	} else {
		// Not done playing, start playing other buffer
		if (numberPlays == 1 &&
		(playingWav->DataPos + AUD_BUF_BYTES > playingWav->DataSize)) {
			// Not repeating another time and audio buffer is larger than
			// remaining number of samples to be played
			// Only play remaining samples, even though buffer is filled
			// as if the file is repeating
			bufferSize = playingWav->DataSize - playingWav->DataPos;
			playingWav->DataPos = playingWav->DataSize;
		} else if (playingWav->DataPos + AUD_BUF_BYTES > playingWav->DataSize) {
			// Remaining samples less than buffer size, but a repeat is coming
			// So start playing the beginning of the new file
			playingWav->DataPos -= (playingWav->DataSize - AUD_BUF_BYTES);
		} else {
			playingWav->DataPos += AUD_BUF_BYTES;
		}
	}

	// Make sure the new data position is valid (non negative)
	// Reset data position to zero if a WAV file is done playing
	if (playingWav->DataPos < 0 || \
	playingWav->DataPos >= playingWav->DataSize || \
	numberPlays == 0) {
		playingWav->DataPos = 0;
	}
	
	// Swap which buffer is playing before playing the new buffer
	toggleBuffers();

	HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*)PLAY_BUFFER,
	bufferSize, DAC_ALIGN_12B_L);
	
	// Set flag so the other buffer can be filled now
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

	/* Open wave data file */
    res = f_open(&F, playingWav->Filename, FA_READ);
	if (res != FR_OK) return res;
	
    /* Jump to correct wave data */
    res = f_lseek(&F, playingWav->SpeechDataOffset + playingWav->DataPos);
	if (res != FR_OK) return res;

	// Read WAV data
	if (playingWav->DataPos + AUD_BUF_BYTES > playingWav->DataSize) {
		// This will reach the end of the file
        res = f_read(&F, READ_BUFFER, playingWav->DataSize, &BytesRead);
		if (res != FR_OK) return res;
        playingWav->DataPos = AUD_BUF_BYTES - playingWav->DataSize;
        // Set file pointer to correct position again
        res = f_lseek(&F, playingWav->SpeechDataOffset + playingWav->DataPos);
		if (res != FR_OK) return res;
        // Read remaining data
        res = f_read(&F, (uint16_t*)((uint8_t*)READ_BUFFER + playingWav->DataSize),
        AUD_BUF_BYTES - playingWav->DataSize, &BytesRead);
		if (res != FR_OK) return res;
	} else {
		// Remaining sample data is of grater size than buffer
		// so read in samples to fill buffer
        res = f_read(&F, READ_BUFFER, AUD_BUF_BYTES, &BytesRead);
		if (res != FR_OK) return res;
        playingWav->DataPos = AUD_BUF_BYTES;
	}

    /* Close file */
    f_close(&F);

	/* Convert 16 Bit Signed to 16 Bit Unsigned */
	ToggleBufferSign((uint32_t*)READ_BUFFER, AUD_BUF_BYTES / 4);

	/* Finished reading into the read buffer, no longer ready to read */
	readyToRead = 0;
	
	return res;
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
uint32_t ReadUnit(uint8_t *buffer, uint8_t idx, uint8_t NbrOfBytes, Endianness BytesFormat)
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
	HAL_TIM_Base_Stop(&htim6);
    HAL_DAC_Stop(&hdac,DAC_CHANNEL_1);
    HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1);

	// Set period based on sample rate
	HAL_TIM_Base_DeInit(&htim6); // deinit
	htim6.Init.Period = W->TIM6ARRValue; // set ARR value
	HAL_TIM_Base_Init(&htim6); // init
	
	// Start TIM6
	HAL_TIM_Base_Start(&htim6);
	// Start DAC
	HAL_DAC_Start(&hdac,DAC_CHANNEL_1);

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
}

/*
 * WAV_Resume is designed so after a file has finished playing a fixed number
 * of times, a call to this function will cause the file to play forever.
 */
void WAV_Resume(void)
{
	/* Enable TIM6 */
	HAL_TIM_Base_Start(&htim6);
}

/**
	* @brief  Executed at each timer interruption (option must be enabled)
	* @param  None
	* @retval None
	*/
void TSL_CallBack_TimerTick(void)
{
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
