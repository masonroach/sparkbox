#include "main.h"

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
void systemInit(void);



int main(void) {
	FRESULT res;                                          /* FatFs function common result code */
	uint32_t byteswritten, bytesread;                     /* File write/read counts */
	uint8_t wtext[] = "Sparkbox's first file!"; /* File write buffer */
	TCHAR fileName[] = "sparkbox.txt"; 				  /* File name */
	uint8_t rtext[100];
	volatile int i;
	FATFS SDFatFs;  /* File system object for SD card logical drive */
	FIL MyFile;     /* File object */
	char SDPath[4]; /* SD card logical drive path */
	
	systemInit();
	
	// FatFs test
	ledAllOff();

	if(FATFS_LinkDriver(&SD_Driver, SDPath) != 0) {goto end;}
	if(f_mount(&SDFatFs, (TCHAR const*)SDPath, 0) != FR_OK) {goto end;}
	if(f_open(&MyFile, fileName, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) {goto end;}
	if(f_write(&MyFile, wtext, sizeof(wtext), (void *)&byteswritten) != FR_OK) {
		f_close(&MyFile); 
		goto end; 
	}
	f_close(&MyFile);
	if(f_open(&MyFile, fileName, FA_READ) != FR_OK) {goto end;}
    if(f_read(&MyFile, rtext, sizeof(rtext), (UINT*)&bytesread) != FR_OK) {
		f_close(&MyFile);
		goto end;
	}
	f_close(&MyFile);
	if((bytesread == byteswritten)){ledError(1); goto end2;} 

end:
	ledError(2);
end2:
	FATFS_UnLinkDriver(SDPath);
		// Push button test
	while(1){
	}
	return 0;
}

void systemInit(void) {
	int8_t i = 0;
	volatile uint16_t j = 0;
	
	/* STM32F4xx HAL library initialization:
       - Configure the Flash prefetch, instruction and Data caches
       - Configure the Systick to generate an interrupt each 1 msec
       - Set NVIC Group Priority to 4
       - Global MSP (MCU Support Package) initialization
     */
	HAL_Init();
  
	/* Configure the system clock to 168 MHz */
	SystemClock_Config();

//	initUsart();
	initLeds();
	initButton();
	initButtons();
//	initSdSpi();
	// RCC->AHB1ENR    |=   RCC_AHB1ENR_GPIODEN;
	// RCC->AHB1ENR    |=   RCC_AHB1ENR_GPIOCEN;
	

	ledError(0);
	
	/*
	 * Initialization is complete. User can press the button to continue at
	 * any time. Until then, a single serial message will be sent, and the
	 * LEDs will continue to light up in a circle.
	 */
//	usartSendString("Initialized. Press button to continue.\r\n");
	while (readButton() == 1) {
	}
	ledAllOff();
}

static void SystemClock_Config(void) {
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Enable HSE Oscillator and activate PLL with HSI as source */
  /* fVCO = fPLLin * (PLLN / PLLM); fPLLout = fVCO / PLLP; fSDIO = fPLLout / PLLQ */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 10; // Clock division (2<=PLLM<=63)
  RCC_OscInitStruct.PLL.PLLN = 210; // Clock multiplication (50<=PLLN<=432)
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2; // PLLP = 2, 4, 6, or 8
  RCC_OscInitStruct.PLL.PLLQ = 10; // 2<=PLLQ<=15 SDIO and RNG clock divider (SDIO<=48MHz)
  // fPLLout = 16e6 * 210 / 10 / 2
  HAL_RCC_OscConfig(&RCC_OscInitStruct);
  
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);

  /* STM32F405x/407x/415x/417x Revision Z devices: prefetch is supported  */
  if (HAL_GetREVID() == 0x1001)
  {
    /* Enable the Flash prefetch */
    __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
  }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line) { 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif
