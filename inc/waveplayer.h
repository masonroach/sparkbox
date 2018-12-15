/*!
 * @file waveplayer.h
 * @author Mason Roach
 * @author Patrick Roy
 * @date Dec 12 2018
 *
 * @brief Functions to control the audio on the Sparkbox
 *
 * These functions are used to play a .WAV file using the FatFs library.
 *
 * Pins in use:
 *
 * | Name | Pin | Use               |
 * |------|-----|:-----------------:|
 * | DAC1 | PA4 | DAC 1 output      |
 * | STBY | PA9 | Audio amp standby |
 *
 *
 */

#ifndef SPARK_WAVEPLAYER
#define SPARK_WAVEPLAYER

#include "stm32f4xx_hal.h"
#include <string.h>
#include "clock.h"
#include "ff.h"

/*! 6ksps ensures no distortion of 3kHz audio (specification max) */
#define SAMPLE_RATE_MIN 6000
/*! DVDs have rate of 48000, rounded to 50000 to support this */
#define SAMPLE_RATE_MAX 50000

/*! Constant for repeating until WAV_Pause() is called */
#define REPEAT_ALWAYS -1

/*! Clock frequency of Timer 6 */
#define TIM6FREQ 84000000UL

/*! Size of the buffer allocated for audio files */
#define AUD_BUF_BYTES 25000
#define AUD_BUF_SAMPLES (AUD_BUF_BYTES / 2)

/*! 16 bit data, left align. 8 bit data, right align */
#define DAC_ADDR (transferSize==BITS_PER_SAMPLE_8 ? \
DAC_ALIGN_12B_R: DAC_ALIGN_12B_L)

/*!
 * @name Defines and Enumerations from STM's waveplayer demo for STM32072B-EVAL,
 * and sparkbox employees claim no credit for them
 * @{
 */

/*!
 * @brief Endianness defines for reading .WAV header
 */
typedef enum
{
	LittleEndian, /*!< Little Endian */
	BigEndian /*!< Big Endian */
} Endianness;

/*!
 * @brief WAV file struct to store all needed information about a WAV file
 */
typedef struct
{
	uint32_t  RIFFchunksize; /*!< Chunk size of header */
	uint16_t  FormatTag; /*!< Contains letters "WAVE" */
	uint16_t  NumChannels; /*!< Mono (1) or Stereo (2) */
	uint32_t  SampleRate; /*!< Sample rate */
	uint32_t  ByteRate; /*!< Bytes per second */
	uint16_t  BlockAlign; /*!< Number of bytes for one sample */
	uint16_t  BitsPerSample; /*!< Bits per sample */
	uint32_t  DataSize; /*!< Size of the data in bytes */
	uint32_t  TIM6ARRValue; /*!< ARR value for Timer 6 based on sample rate */
	uint32_t  SpeechDataOffset; /*!< Offset from beginning of file to data */
	char      Filename[64]; /*!< Filename associated with the WAV file */
	uint16_t  Error; /*!< Current error status of the WAV file */
} WAV_Format;

/*!
 * @brief Enumeration for WAV file error codes
 */
typedef enum
{
	Valid_WAVE_File = 0, /*!< No error */
	Bad_RIFF_ID, /*!< "RIFF" text invalid */
	Bad_WAVE_Format, /*!< "WAVE" text invalid */
	Bad_FormatChunk_ID, /*!< "fmt" text invalid */
	Bad_FormatTag, /*!< Compressed audio not supported */
	Bad_Number_Of_Channel, /*!< Only mono audio is supported */
	Bad_Sample_Rate, /*!< Sample rate out*/
	Bad_Bits_Per_Sample, /*!< */
	Bad_DataChunk_ID, /*!< */
	Bad_ExtraFormatBytes, /*!< */
	Bad_FactChunk_ID, /*!< "FACT" text invalid */
	Bad_DataSize, /*!< Data cannot fit in allocated memory */
	Bad_FileRead /*!< Error reading file */
} ErrorCode;

/* Correspond to the letters 'RIFF' */
#define CHUNK_ID 0x52494646
/* Correspond to the letters 'WAVE' */
#define FILE_FORMAT 0x57415645
/* Correspond to the letters 'fmt ' */
#define FORMAT_ID 0x666D7420
/* Correspond to the letters 'data' */
#define DATA_ID 0x64617461
/* Correspond to the letters 'fact' */
#define FACT_ID 0x66616374
/* PCM of 1 indicates no compression of the data */
#define WAVE_FORMAT_PCM 0x01
/* The format chunk size is 16 for PCM of 1 */
#define FORMAT_CHNUK_SIZE 0x10
/* Mono and Stereo */
#define CHANNEL_MONO 0x01
#define CHANNEL_STEREO 0x02
#define BITS_PER_SAMPLE_8 8
#define BITS_PER_SAMPLE_16 16

/* @} */


/*!
 * @brief Initializes wave player and allocates memory for audio buffer
 */
void WAV_Init(void);

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
uint8_t WAV_Import(const char* FileName, WAV_Format* W);

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
void WAV_Play(WAV_Format* W, int numPlays);

/*!
 * @brief Pauses the currently playing .WAV file and turns off the audio amp
 */
void WAV_Pause(void);

/*!
 * @brief Resumes playing the imported .WAV file and turns on the audio amp
 */
void WAV_Resume(void);

/*!
 * @brief Stops playing the .WAV file and deinitializes the .WAV peripherals
 */
void WAV_Destroy(void);

#endif
