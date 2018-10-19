#ifndef sparkboxBootloader
#define sparkboxBootloader

#include "ff.h"
#include "stm32f4xx.h"

/* Module defines */
#define BOOTLOADER_SIZE_BYTES 32000
#define PROGRAM_FLASH_START (FLASH_BASE + BOOTLOADER_SIZE_BYTES)

/* Error codes for this module */
enum flashErrorCode {
	FLASH_SUCCESS = 0,
	PROGRAM_TOO_BIG,
	BAD_FLASH_WRITE
};

/* Public function headers */
UINT fatFsProgramFlash(char* filename, uint32_t *programAddr);
void executeProgram (uint32_t addr);

#endif
