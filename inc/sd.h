#include "stm32f3xx.h"

#ifndef SPARK_SD
#define SPARK_SD

#ifndef CLOCK_F
#define CLOCK_F 8000000UL
#endif

/*
 * List of commands for SD cards
 */
typedef enum {
	GO_IDLE_STATE 			= 0,	// Resets the SD memory card
	SEND_OP_COND 			= 1,	// Sends host capacity support information
	SWITCH_FUNC 			= 6,	// Switches card function
	SEND_IF_COND 			= 8,	// Sends SD card interface condition
	SEND_CSD 				= 9,	// Asks card to send its card-specific data
	SEND_CID 				= 10,	// Asks card to send its card identification
	STOP_TRANSMISSION		= 12,	// Stop transmission in multi-block read
	SEND_STATUS 			= 13,	// Asks card to send its status register
	SET_BLOCKLEN			= 16,	// Set [blocksize] for following block cmds
	READ_SINGLE_BLOCK		= 17,	// Reads a block of size [blocksize]
	READ_MULTIPLE_BLOCK		= 18,	// Continuously reads until CMD12
	WRITE_BLOCK				= 24,	// Writes a block of size [blocksize]
	WRITE_MULTIPLE_BLOCK	= 25,	// Continuously writes until CMD12
	PROGRAM_CSD				= 27,	// Flash programmable bits of the CSD
	ERASE_WR_BLK_START_ADDR	= 32,	// Sets address of first block erased
	ERASE_WR_BLK_END_ADDR	= 33,	// Sets address of last block erased
	ERASE					= 38,	// Erased previously selected write blocks
	LOCK_UNLOCK				= 42,	// Lock or unlock the card
	APP_CMD					= 55,	// Defines the next cmd is an application
	GEN_CMD					= 56,	// Transfer data block or get a data block
	READ_OCR				= 58,	// Reads the OCR register
	CRC_ON_OFF				= 59	// Toggles the CRC option
} SDCOMMAND;

/*
 * Types of CRCs that can get fetched
 */
typedef enum {
	SPI_CRC_RX,		// Receive CRC
	SPI_CRC_TX		// Transmit CRC
} CRCTYPE;

void csHigh(void);
void csLow(void);
void sdInit(void);
void sdSpiPinInit(void);
void sdSendCmd(SDCOMMAND cmd, uint32_t args);
uint8_t sdSendByte(uint8_t byte);
uint16_t sdGetCRC(CRCTYPE crcType);
uint8_t sdReadByte(void);

#endif
