#include "bootloader.h"

/* Private function headers */

// Contains the next unused flash memory address
uint32_t flashAddr = PROGRAM_FLASH_START;

/* Program the flash from fatfs */
UINT fatFsProgramFlash(char* filename, uint32_t *programAddr)
{
	FIL binFile;
	UINT fres;
	HAL_StatusTypeDef status;

	uint32_t *bytesRead = NULL;
	uint8_t buffer;
	
	// Open the file
	fres = f_open(&binFile, filename, FA_READ);
	if (fres != FR_OK) return fres;

	// Check if enough memory exists for the flash (file size in bytes)
	if (flashAddr + f_size(&binFile) > FLASH_END) return PROGRAM_TOO_BIG;

	// S
	*programAddr = flashAddr;

	// Copy file into flash memory while not at the end of the file
	while (!f_eof(&binFile)) {
		// Read a byte
		fres = f_read(&binFile, &buffer, 8, (UINT*)bytesRead);

		// Check for file read error
		if (fres != FR_OK) {
			f_close(&binFile);
			return fres;
		}
		
		// Write a byte
		status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, (uint32_t)flashAddr,
			(uint64_t)buffer);

		// Check for flash write error
		if (status != HAL_OK) {
			f_close(&binFile);
			return BAD_FLASH_WRITE;
		}
	}
	return FLASH_SUCCESS;

}

/* Executes a program at a given address in flash */
void executeProgram (uint32_t addr)
{
	// This will fail if the user has not yet programmed the flash
	// the user is attempting to jump to
	if (addr < FLASH_BASE || addr > flashAddr) return;

	// Cast addr to a no arg function and run it
	((void (*)(void)) addr)();	

	return;
}
