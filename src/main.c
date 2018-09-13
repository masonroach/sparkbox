#include "main.h"

static void SystemClock_Config(void);

int main(void)
{
	volatile uint32_t i;
	uint8_t j = 0, error = 0;
	uint32_t byteswritten, bytesread;
	FATFS SDFatFs;  /* File system object for SD card logical drive */
	FIL MyFile;     /* File object */
	char SDPath[4]; /* SD card logical drive path */
	uint8_t wtext[] = "Sparkbox works on the PCB!";
	TCHAR fileName[] = "sparkboxpcb.txt";
	uint8_t rtext[100];

/*
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;

	GPIOB->MODER |= (GPIO_MODER_MODE2_0);
	GPIOC->MODER |= (GPIO_MODER_MODE0_0);
*/
	HAL_Init();
	SystemClock_Config();
	initLeds();
	ledAllOff();

	for (i= 0; i < 1000000; i++);

	if(FATFS_LinkDriver(&SD_Driver, SDPath) != 0) {ledOn(1); goto end;}
	if(f_mount(&SDFatFs, (TCHAR const*)SDPath, 0) != FR_OK) {ledOn(2); goto end;}
	if(f_open(&MyFile, fileName, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) {ledOn(3); goto end;}
	if(f_write(&MyFile, wtext, sizeof(wtext), (void *)&byteswritten) != FR_OK) {
		f_close(&MyFile);
		ledOn(4);
		goto end;
	}
	f_close(&MyFile);

	if(f_open(&MyFile, fileName, FA_READ) != FR_OK) {goto end;}
	if(f_read(&MyFile, rtext, sizeof(rtext), (UINT*)&bytesread) != FR_OK) {
		f_close(&MyFile);
		goto end;
	}
	f_close(&MyFile);
	if((bytesread == byteswritten)) ledOn(0);

end:
	FATFS_UnLinkDriver(SDPath);

	// while(1){
/*
		GPIOB->ODR ^= GPIO_ODR_OD2;
		GPIOC->ODR ^= GPIO_ODR_OD0;
*/
	//	if (++j > 8) {
	//		j = 0;
	//		error = ++error > 2 ? 0 : error;
	//		ledError(error);
	//	}
	//	ledMap(0xFF >> (8 - j));
	//	for(i=0; i < 100000; i++);
	// }
	while(1);
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

  	/* Enable HSE Oscillator and activate PLL with HSE as source */
  	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  	RCC_OscInitStruct.PLL.PLLM = 25;
  	RCC_OscInitStruct.PLL.PLLN = 336;
  	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  	RCC_OscInitStruct.PLL.PLLQ = 7;
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

