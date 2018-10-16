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
static void WavePlayer_ReadAndParse(const char* WavName, WAV_Format* WAVE_Format);
static void ToggleBufferSign(uint32_t* pBuffer, uint32_t BufferSize);
uint32_t ReadUnit(uint8_t *buffer, uint8_t idx, uint8_t NbrOfBytes, Endianness BytesFormat);
/**
  * @}
  */

// Handles for initialization
DAC_HandleTypeDef hdac;
DMA_HandleTypeDef hdma_dac1;
TIM_HandleTypeDef htim6;

int numberPlays;
FIL F;

/** @defgroup WAVEPLAYER_Private_Functions
  * @{
  */

/**
  * @brief  Wave player Initialization
  * @param  None
  * @retval None
  */
void WAV_Init(void)
{
	DAC_ChannelConfTypeDef sConfig;
	TIM_MasterConfigTypeDef sMasterConfig;

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/********************************* DMA Config *******************************/
    /* DMA1 clock enable (to be used with DAC) */
    __HAL_RCC_DMA1_CLK_ENABLE();
    /* DMA channel3 Configuration is managed by WAV_Play() function */


    /* Enable the DMA IRQ --------------------------------------*/
    NVIC_SetPriority(DMA1_Stream5_IRQn, 0x40);
    NVIC_EnableIRQ(DMA1_Stream5_IRQn);

    /* Enable Transfer Complete Interrupt on channel 5 */
    //DMA1_Stream5->CR |= DMA_SxCR_TCIE;


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

/**
	* @brief  Checks the format of the .WAV file and gets information about the audio
	*         format. This is done by reading the value of a number of parameters
	*         stored in the file header and comparing these to the values expected
	*         authenticates the format of a standard .WAV  file (44 bytes will be read).
	*         If  it is a valid .WAV file format, it continues reading the header
	*         to determine the  audio format such as the sample rate and the sampled
	*         data size. If the audio format is supported by this application, it
	*         retrieves the audio format in WAVE_Format structure and returns a zero
	*         value. Otherwise the function fails and the return value is nonzero.
	*         In this case, the return value specifies the cause of  the function
	*         fails. The error codes that can be returned by this function are declared
	*         in the header file.
	* @param  WavName: wav file name
	* @param  FileLen: wav file length   
	* @retval Zero value if the function succeed, otherwise it returns a nonzero 
	*         value which specifies the error code.
	*/
void WavePlayer_ReadAndParse(const char* WavName, WAV_Format* WAVE_Format)
{
	UINT BytesRead;
	uint32_t temp = 0x00;
	uint32_t extraformatbytes = 0;
	uint16_t TempBuffer[_MAX_SS];
	uint8_t i, res;
	
	f_open(&F, WavName, FA_READ);

	res = f_read(&F, TempBuffer, _MAX_SS, &BytesRead);

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

	/* Read the number of channels, must be 0x01 (Mono) or 0x02 (Stereo) ----------------------*/
	WAVE_Format->NumChannels = ReadUnit((uint8_t*)TempBuffer, 22, 2, LittleEndian);
	
	/*
	if(WAVE_Format->NumChannels != WAVEPLAYER_CHANNEL){
		f_close(&F);
		WAVE_Format->Error = Unsupporetd_Number_Of_Channel;
		return(&WAVE_Format);
	}
	*/

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
	
	if(WAVE_Format->BitsPerSample != BITS_PER_SAMPLE_16){
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
	// WaveCounter =  SpeechDataOffset;
	f_close(&F);
	WAVE_Format->Error = Valid_WAVE_File;
        return;
}

/**
	* @brief  Reads .WAV file into memory
	* @param  FileName: pointer to the wave file name to be read
	* @param  FileLen: pointer to the wave struct
	* @param  Buffer: pointer to the memory where WAV data is DMA accessible
	* @retval None
	*/
uint8_t WAV_Import(const char* FileName, WAV_Format* W, uint32_t* Buffer)
{
	UINT BytesRead;

	/* Read the Speech wave file status */
	WavePlayer_ReadAndParse((const char*)FileName, W);

	if(W->Error != Valid_WAVE_File){
		return W->Error;
	}
	
	/* Open wave data file */
	f_open(&F, FileName, FA_READ);
	
	/* Jump to wave data */
	f_lseek(&F, W->SpeechDataOffset);

	/* Store data in buffer selected by user */
 	f_read(&F, Buffer, W->DataSize, &BytesRead);

	/* Close file */
	f_close(&F);

	/* Convert 16 Bit to 12 Bit format */
	ToggleBufferSign(Buffer, W->DataSize / 4);

	return 0;
}  
	
/**
	* @brief  DMA transfer complete 
	* @param  None
	* @retval None
	*/
void DMA1_Stream5_IRQHandler(void)
{
	HAL_DMA_IRQHandler(&hdma_dac1);
	
	// If the last play has occurred, stop
	if (numberPlays == 1) HAL_TIM_Base_Stop(&htim6);
	else if (numberPlays <= REPEAT_ALWAYS) numberPlays = REPEAT_ALWAYS;
	else numberPlays--;
	// If DMA transfer is complete
	//if(DMA1->HISR & DMA_HISR_TCIF5) {
		// Clear transfer complete bit (writing 1 clears bit)
	//	DMA1->HIFCR |= DMA_HIFCR_CTCIF5;

		// Clear control register
		// DMA1_Stream5->CR = 0x0;

		/* Disable TIM6 counter */
        // TIM6->CR1 &= ~TIM_CR1_CEN;
		
		// Disable TIM6 counter if done playing
		// if (numberPlays != REPEAT_ALWAYS && --numberPlays == 0)
    	//	HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1);
		
	//}
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
	* @brief  Starts playing audio stream from the audio Media.
	* @param  Addr: Buffer address 
	* @param  Size: Buffer size  
	* @retval None
	*/
void WAV_Play(uint32_t Addr, WAV_Format* W, int numPlays)
{
	
	// Do not play if number of plays is 0
	if (numPlays == 0) return;

	// If numPlays is negative, set it to REPEAT_ALWAYS (-1)
	// This allows interrupt to call this function with a decrement
	if (numPlays < 0) numPlays = REPEAT_ALWAYS;
	
	/* Save data for DMA interrupt */
	numberPlays = numPlays;

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
	// Start DAC with DMA
	HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*)Addr,
		(uint32_t)(W->DataSize / 2), DAC_ALIGN_12B_L);

}

void WAV_Pause(void)
{
	/* Disable TIM6 */
	HAL_TIM_Base_Stop(&htim6);
}

void WAV_Resume(void)
{
	/* Enable TIM6 if number of plays is nonzero */
	if (numberPlays != 0) HAL_TIM_Base_Start(&htim6);
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
