#include "main.h"

FATFS SDFatFs;  /* File system object for SD card logical drive */
char SDPath[4]; /* SD card logical drive path */

// This is the main function for the bootloader module
int main(void)
{
	uint32_t addr;
	DIR dp;
	// The ? matches any char and the
	char *pattern = "?*.bin";
	FILINFO fno;
	FRESULT fr;

	/* STM32F4xx HAL library initialization:
       - Configure the Flash prefetch, instruction and Data caches
       - Configure the Systick to generate an interrupt each 1 msec
       - Set NVIC Group Priority to 4
       - Global MSP (MCU Support Package) initialization
     */
	HAL_Init();

	/* Configure the system clock to 168 MHz */
    initSystemClock();

	initLeds();

	ledAllOn();

	/* Set the pattern to search for later on */
	dp.pat = pattern;

	/* Configure FatFS to use the uSD card 
	 * Dead loop on any error configuring the driver or mounting
	 */
	if (FATFS_LinkDriver(&SD_Driver, SDPath) != 0) while (1);
	if (f_mount(&SDFatFs, (TCHAR const*)SDPath, 0) != FR_OK) while (1);


	while (1) {
		// Open root directory
		fr = f_opendir(&dp, "");
	
		// If opening the directory failed, try again
		if (fr != FR_OK) continue;

		// Find the first file that matches the pattern
		fr = f_findnext(&dp, &fno);

		if (fr != FR_OK) {
			// If an error occurred, close directory and try again
			f_closedir(&dp);
			continue;
		}

		// Check if a valid file was found
		if (fno.fname[0]) {
			// If it can be run, copy it to flash memory
			if (!fatFsProgramFlash(fno.fname, &addr)) {
				ledAllOff();
				// execute the program on successful copy
				executeProgram(addr);
			}
			f_closedir(&dp);
			continue;
		} else {
			// If no valid file is found, close directory and try again
    	    f_closedir(&dp);
    	    continue;
		}
	}

}

