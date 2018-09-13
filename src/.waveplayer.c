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

/** @defgroup WAVEPLAYER
  * @{
  */

/**
  * @}
  */

/** @defgroup WAVEPLAYER_Private_Macros
  * @{
  */
/**
  * @}
  */

/** @defgroup WAVEPLAYER_Private_Variables
  * @{
  */
WAVE_FormatTypeDef WAVE_Format;
uint16_t TIM6ARRValue = 6000;
__IO uint32_t WaveDataLength = 0;
// extern __IO uint32_t WaveCounter;
static __IO uint32_t SpeechDataOffset = 0x00;
static uint32_t wavelen = 0;
extern uint32_t Buffer1[BUFFER_SIZE_WORD];
extern uint32_t Buffer2[BUFFER_SIZE_WORD];
extern FATFS fs;
// extern DIR dir;
// extern char* DirectoryFiles[MAX_FILES];
// extern __IO uint8_t NumberOfFiles;
// extern uint32_t bmplen;
// extern WavePlayList PlayList[MAX_FILES];
extern FIL F;
extern UINT BytesWritten;
extern UINT BytesRead;
__IO uint32_t BufferSelection = 1;
__IO uint32_t BufferRead = 0;

/**
  * @}
  */

/** @defgroup WAVEPLAYER_Private_FunctionPrototypes
  * @{
  */

static ErrorCode WavePlayer_ReadAndParse(const char* WavName, uint32_t *FileLen);
static void ToggleBufferSign(uint32_t* pBuffer, uint32_t BufferSize);
static void Audio_MAL_Play(uint32_t Addr, uint32_t Size);
/**
  * @}
  */

/** @defgroup WAVEPLAYER_Private_Functions
  * @{
  */

/**
  * @brief  Wave player Initialization
  * @param  None
  * @retval None
  */
void WavePlayer_Init(void)
{
  uint8_t tmppriority = 0x00, tmppre = 0x00, tmpsub = 0x0F;
  // DAC_InitTypeDef  DAC_InitStructure;
  // GPIO_InitTypeDef GPIO_InitStructure;
  // NVIC_InitTypeDef NVIC_InitStructure;
  

  /******************************* TIM6 Configuration *************************/
   
  /* TIM6 clock enable */
  RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
  
  /* Reset Timer 6 */
  RCC->APB1RSTR |= RCC_APB1RSTR_TIM6RST;
  RCC->APB1RSTR &= ~RCC_APB1RSTR_TIM6RST;
  
  /* Set the timer period */
  TIM6->ARR = TIM6ARRValue;
  
  /* TIM6 TRGO update selection */
  TIM6->CR2 &= (uint16_t)~TIM_CR2_MMS;
  TIM6->CR2 |=  TIM_CR2_MMS_1;
  
  /******************************* Init DAC channel 1 (PA4) *******************/  
  /* DAC Periph clock enable */
  RCC->APB1ENR |= RCC_APB1ENR_DACEN;
  
  /* Reset DAC */
  RCC->APB1RSTR |= RCC_APB1RSTR_DACRST;
  RCC->APB1RSTR &= ~RCC_APB1RSTR_DACRST;
  
  /* GPIOA clock enable */
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; 
  
  /* Configure PA.04 (DAC_OUT1) in analog mode, No PUPD */
  GPIOA->MODER |= GPIO_MODER_MODE4;
  GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD4;
  
  /* Clear MAMP1, WAVE1, TSEL1, TEN1, BOFF1 bits */
  DAC->CR &= ~(DAC_CR_MAMP1 | DAC_CR_WAVE1 | DAC_CR_TSEL1 | DAC_CR_TEN1 | DAC_CR_BOFF1);
  /* TSEL1 = 0b000, TIM6 trigger source */
  /* DAC Channel 1 Trigger Enable */
  DAC->CR |= DAC_CR_TEN1;
  
  /* Enable DAC Channel 1 */
  DAC->CR |= DAC_CR_EN1;
  
  /************************** Init DAC channel 2 (PA5) ************************/
	// /* Configure PA.05 (DAC_OUT2) in analog mode, No PUPD */
	// GPIOA->MODER |= GPIO_MODER_MODE5;
	// GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD5;
	
	// /* Clear MAMP2, WAVE2, TSEL2, TEN2, BOFF2 bits */
	// DAC->CR &= ~(DAC_CR_MAMP2 | DAC_CR_WAVE2 | DAC_CR_TSEL2 | DAC_CR_TEN2 | DAC_CR_BOFF2);

	// /* TSEL2 = 0b000, TIM6 trigger source */
	// /* DAC Channel 2 Trigger Enable */
	// DAC->CR |= DAC_CR_TEN2;
	
	// /* Enable DAC Channel 2 */
	// DAC->CR |= DAC_CR_EN2;
  
  /********************************* DMA Config *******************************/
  /* DMA1 clock enable (to be used with DAC) */
  RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
  /* DMA channel3 Configuration is managed by Audio_MAL_Play() function */

  
  // From misc.c NVIC _Init() in standard peripheral libraries
  /* Compute the Corresponding IRQ Priority --------------------------------*/    
    tmppriority = (0x700 - ((SCB->AIRCR) & (uint32_t)0x700))>> 0x08;
    tmppre = (0x4 - tmppriority);
    tmpsub = tmpsub >> tmppriority;

    tmppriority = 0 << tmppre;
    tmppriority = tmppriority << 0x04;
        
    NVIC->IP[DMA1_Stream3_IRQn] = tmppriority;
    
    /* Enable the Selected IRQ Channels --------------------------------------*/
    NVIC->ISER[DMA1_Stream3_IRQn >> 0x05] =
      (uint32_t)0x01 << (DMA1_Stream3_IRQn & (uint8_t)0x1F);
  
  /* Enable Transfer Complete Interrupt on channel 3 */
  DMA1_Stream3->CR |= DMA_SxCR_TCIE;
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
static ErrorCode WavePlayer_ReadAndParse(const char* WavName, uint32_t *FileLen)
{
	uint32_t temp = 0x00;
	uint32_t extraformatbytes = 0;
	uint16_t TempBuffer[_MAX_SS];
  
	f_open(&F, WavName, FA_READ);

	f_read(&F, TempBuffer, _MAX_SS, &BytesRead);

	/* Read chunkID, must be 'RIFF'  -------------------------------------------*/
	temp = ReadUnit((uint8_t*)TempBuffer, 0, 4, BigEndian);
	if(temp != CHUNK_ID){
		f_close(&F);
		return(Unvalid_RIFF_ID);
	}

	/* Read the file length ----------------------------------------------------*/
	WAVE_Format.RIFFchunksize = ReadUnit((uint8_t*)TempBuffer, 4, 4, LittleEndian);

	/* Read the file format, must be 'WAVE' ------------------------------------*/
	temp = ReadUnit((uint8_t*)TempBuffer, 8, 4, BigEndian);
	if(temp != FILE_FORMAT){
		f_close(&F);
		return(Unvalid_WAVE_Format);
	}

	/* Read the format chunk, must be'fmt ' ------------------------------------*/
	temp = ReadUnit((uint8_t*)TempBuffer, 12, 4, BigEndian);
	if(temp != FORMAT_ID){
		f_close(&F);
		return(Unvalid_FormatChunk_ID);
	}
	/* Read the length of the 'fmt' data, must be 0x10 -------------------------*/
	temp = ReadUnit((uint8_t*)TempBuffer, 16, 4, LittleEndian);
	if(temp != 0x10){
		extraformatbytes = 1;
	}
	/* Read the audio format, must be 0x01 (PCM) -------------------------------*/
	WAVE_Format.FormatTag = ReadUnit((uint8_t*)TempBuffer, 20, 2, LittleEndian);
	if(WAVE_Format.FormatTag != WAVE_FORMAT_PCM){
		f_close(&F);
		return(Unsupporetd_FormatTag);
	}

	/* Read the number of channels, must be 0x01 (Mono) or 0x02 (Stereo) ----------------------*/
	WAVE_Format.NumChannels = ReadUnit((uint8_t*)TempBuffer, 22, 2, LittleEndian);
  
	if(WAVE_Format.NumChannels != WAVEPLAYER_CHANNEL){
		f_close(&F);
		return(Unsupporetd_Number_Of_Channel);
	}

	/* Read the Sample Rate ----------------------------------------------------*/
	WAVE_Format.SampleRate = ReadUnit((uint8_t*)TempBuffer, 24, 4, LittleEndian);
	/* Update the OCA value according to the .WAV file Sample Rate */
	switch (WAVE_Format.SampleRate){
		case SAMPLE_RATE_8000 :
			TIM6ARRValue = 6000;
			break; /* 8KHz = 48MHz / 6000 */
		case SAMPLE_RATE_16000 :
			TIM6ARRValue = 3000;
			break; /* 8KHz = 48MHz / 6000 */
		case SAMPLE_RATE_11025:
			TIM6ARRValue = 4353;
			break; /* 11.025KHz = 48MHz / 4353 */
		case SAMPLE_RATE_22050:
			TIM6ARRValue = 2176;
			break; /* 22.05KHz = 48MHz / 2176 */
		case SAMPLE_RATE_32000:
			TIM6ARRValue = 1500;
			break; /* 32KHz = 48MHz / 1500 */
		case SAMPLE_RATE_44100:
			TIM6ARRValue = 1088;
			break; /* 44.1KHz = 48MHz / 1088 */
		default:
			f_close(&F);
			return(Unsupporetd_Sample_Rate);
	}

	/* Read the Byte Rate ------------------------------------------------------*/
	WAVE_Format.ByteRate = ReadUnit((uint8_t*)TempBuffer, 28, 4, LittleEndian);

	/* Read the block alignment ------------------------------------------------*/
	WAVE_Format.BlockAlign = ReadUnit((uint8_t*)TempBuffer, 32, 2, LittleEndian);

	/* Read the number of bits per sample --------------------------------------*/
	WAVE_Format.BitsPerSample = ReadUnit((uint8_t*)TempBuffer, 34, 2, LittleEndian);
  
	if(WAVE_Format.BitsPerSample != BITS_PER_SAMPLE_16){
		f_close(&F);
		return(Unsupporetd_Bits_Per_Sample);
	}
	SpeechDataOffset = 36;
	/* If there is Extra format bytes, these bytes will be defined in "Fact Chunk" */
	if(extraformatbytes == 1){
		/* Read th Extra format bytes, must be 0x00 ------------------------------*/
		temp = ReadUnit((uint8_t*)TempBuffer, 36, 2, LittleEndian);
		if(temp != 0x00){
			f_close(&F);
			return(Unsupporetd_ExtraFormatBytes);
		}
		/* Read the Fact chunk, must be 'fact' -----------------------------------*/
		temp = ReadUnit((uint8_t*)TempBuffer, 38, 4, BigEndian);
		if(temp != FACT_ID){
			f_close(&F);
			return(Unvalid_FactChunk_ID);
		}
		/* Read Fact chunk data Size ---------------------------------------------*/
		temp = ReadUnit((uint8_t*)TempBuffer, 42, 4, LittleEndian);
		SpeechDataOffset += 10 + temp;
	}
	/* Read the Data chunk, must be 'data' -------------------------------------*/
	temp = ReadUnit((uint8_t*)TempBuffer, SpeechDataOffset, 4, BigEndian);
	SpeechDataOffset += 4;
	if(temp != DATA_ID){
		f_close(&F);
		return(Unvalid_DataChunk_ID);
	}

	/* Read the number of sample data ------------------------------------------*/
	WAVE_Format.DataSize = ReadUnit((uint8_t*)TempBuffer, SpeechDataOffset, 4, LittleEndian);
	SpeechDataOffset += 4;
	// WaveCounter =  SpeechDataOffset;
	f_close(&F);
	return(Valid_WAVE_File);
}

/**
  * @brief  Starts playing wave.
  * @param  FileName: pointer to the wave file name to be read
  * @param  FileLen: pointer to the wave file length to be read
  * @retval None
  */
uint8_t WavePlayerMenu_Start(const char* FileName, uint32_t *FileLen)
{
  
  /* Read the Speech wave file status */
  if(WavePlayer_ReadAndParse((const char*)FileName, &wavelen) == Valid_WAVE_File)
  {
    /* Set WaveDataLenght to the Speech wave length */
    WaveDataLength = WAVE_Format.DataSize;
  }
  else
  {
    return 0;
  }
     
  WavePlayer_Init();
  
  /* Jump to wave data */
  f_lseek(&F, SpeechDataOffset);
  
  /* Store data in buffer 1 */
  f_read(&F, Buffer1, BUFFER_SIZE_BYTE, &BytesRead);

  /* Convert signed to unsigned format */
  ToggleBufferSign(Buffer1, BUFFER_SIZE_WORD);

  /* Play buffer 1 */
  Audio_MAL_Play((uint32_t)Buffer1, BUFFER_SIZE_WORD);
  
  /* Store data in buffer 2 */
  f_read(&F, Buffer2, BUFFER_SIZE_BYTE, &BytesRead);
  
  /* Invert sign bit: PCM format is 16-bit signed and DAC is 12-bit unsgined */
  ToggleBufferSign(Buffer2, BUFFER_SIZE_WORD);

  
  BufferSelection = 2;
  WaveDataLength = WAVE_Format.DataSize;
  /* while not end of file */
  while (F.fptr < (F.fsize - 2*BUFFER_SIZE_BYTE))
  {
    if(BufferRead != 0)
    {
      if(BufferSelection == 2)
      {
        /* Play buffer 2 */
        Audio_MAL_Play((uint32_t)Buffer2, BUFFER_SIZE_WORD);
        /* Store data in buffer 1 */
        if(f_read(&F, Buffer1, BUFFER_SIZE_BYTE, &BytesRead) != FR_OK)
        {
          while(1);
        }
        
        /* Convert signed to unsigned format */
        ToggleBufferSign(Buffer1, BUFFER_SIZE_WORD);
        
        BufferRead = 0;
      }
      else if (BufferSelection == 1)
      {
        /* Play buffer 1 */
        Audio_MAL_Play((uint32_t)Buffer1, BUFFER_SIZE_WORD);
        /* Store data in buffer 2 */  
        if(f_read(&F, Buffer2, BUFFER_SIZE_BYTE, &BytesRead) != FR_OK)
        {
          while(1);
        }
        
        /* Convert signed to unsigned format */
        ToggleBufferSign(Buffer2, BUFFER_SIZE_WORD);
        
        BufferRead = 0;
      }
      WaveDataLength -= BUFFER_SIZE_BYTE;
    }
  }
  
  DMA1_Stream3->CR = 0x0;

  /* Disable TIM6 */
  TIM_Cmd(TIM6, DISABLE);
  
  WaveDataLength = 0;
  
  /* Close file */
  f_close(&F);
  
  return 0;
}  
  
/**
  * @brief  DMA transfer complete 
  * @param  None
  * @retval None
  */
void DMA1_Stream3_IRQHandler(void)
{
	// If DMA transfer is complete
	if(DMA1->LISR & DMA_LISR_TCIF3) {
		// Clear transfer complete bit (writing 1 clears bit)
		DMA1->LIFCR |= DMA_LIFCR_CTCIF3;
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
uint32_t ReadUnit(uint8_t *buffer, uint8_t idx, uint8_t NbrOfBytes, Endianness BytesFormat)
{
  uint32_t index = 0;
  uint32_t temp = 0;

  for (index = 0; index < NbrOfBytes; index++)
  {
    temp |= buffer[idx + index] << (index * 8);
  }

  if(BytesFormat == BigEndian)
  {
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
  uint32_t loopcounter = 0;
  
  /* Invert sign bit: PCM format is 16-bit signed and DAC is 12-bit unsigned */
  for(loopcounter = 0; loopcounter < BufferSize; loopcounter++)
  {
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
static void Audio_MAL_Play(uint32_t Addr, uint32_t Size)
{
  /* Disable DMA for DAC Channel 1 */
  DAC->CR &= ~DAC_CR_DMAEN1;
  
  /* Disable DMA1 Channel3 */
  DMA1_Stream3->CR &= ~DMA_SxCR_EN;
  
  /* Disable TIM6 counter */
  TIM6->CR1 &= ~TIM_CR1_CEN;
  
  /* Clear CHSEL, MBURST, PBURST, PL, MSIZE, PSIZE, MINC, PINC, CIRC and DIR bits */
  DMA1_Stream3->CR &= ((uint32_t)~(DMA_SxCR_CHSEL | DMA_SxCR_MBURST | DMA_SxCR_PBURST |
                         DMA_SxCR_PL | DMA_SxCR_MSIZE | DMA_SxCR_PSIZE |
                         DMA_SxCR_MINC | DMA_SxCR_PINC | DMA_SxCR_CIRC |
                         DMA_SxCR_DIR));
						 
  /* Set bit values needed to complete DMA transfer */
  /*
  * Memory to Peripheral
  * PeriphSIZE and MemSIZE = word (32 bits)
  * Only memory address increments every iteration
  * High priority level
  */
  DMA1_Stream3->CR |= (DMA_SxCR_DIR_0 | DMA_SxCR_MINC | DMA_SxCR_PSIZE_1
					   DMA_SxCR_MSIZE_1 | DMA_SxCR_PL_1);					 
					

  /*------------------------- DMAy Streamx NDTR Configuration ----------------*/
  /* Write to DMAy Streamx NDTR register */
  DMA1_Stream3->NDTR = (uint32_t)Size;

  /*------------------------- DMAy Streamx PAR Configuration -----------------*/
  /* Write to DMAy Streamx PAR */
  DMA1_Stream3->PAR = WAVEPLAYER_DAC_ADDRESS;

  /*------------------------- DMAy Streamx M0AR Configuration ----------------*/
  /* Write to DMAy Streamx M0AR */
  DMA1_Stream3->M0AR = (uint32_t)Addr;
  
  /* Enable the DMA Channel */
  DMA1_Stream3->CR |= DMA_SxCR_EN;
  
  /* Enable DMA for DAC Channel 1 */
  DAC->CR |= DAC_CR_DMAEN1;
  
  /* Enable TIM6 counter */
  TIM6->CR1 |= TIM_CR1_CEN;
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