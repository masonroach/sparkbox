#include "main.h"

void systemInit(void);
static void SystemClock_Config(void);

int main(void) {
	uint16_t i = 0;

	// Initialize peripherals
	systemInit();
	
	// Main code
	lcdTest();

	// End with a dead loop
	while (1);

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
	if((bytesread != byteswritten)){goto end;}


	
	// SUCCESS, let's play a WAV file
	ledError(1);

	// PLAY WAV FILE HERE

	goto end2;

end:
	ledError(2);
end2:
	FATFS_UnLinkDriver(SDPath);
	// Push button test
	while(1);

	return 0;
	
}

void systemInit(void) {
	int8_t i = 0;
	int8_t e = 0;
	volatile uint32_t j = 0;

	SystemClock_Config();
	initLeds();
	initButton();
	initLcd();

	ledError(LED_OFF);
	while (readButton()) {
		i > 8 ? i = 0 : i++;
		ledMap((0xFF >> (8-i)) & 0xFF);
//		ledMap(0xFF & rand32());
		ledError(e > 2 ? e = 0 : e++);
		for (j = 0; j < 500000; j++);
	}
	while (readButton() == 1);
	ledAllOff();
	ledError(LED_OFF);
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

void lcdTest(void) {
//	LcdWriteCmd(0xFFFF);
	while (1) {
//		LcdWriteCmd(i++);
		LcdWriteCmd(1);
		LcdWriteCmd(0);
		LcdWriteCmd(1);
		LcdWriteCmd(0);
		LcdWriteCmd(1);
		while (readButton());
		while (!readButton());
	}
}
