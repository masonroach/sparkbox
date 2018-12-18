/*!
 * @file clock.c
 * @author Mason Roach
 * @author Patrick Roy
 * @date Dec 13 2018
 *
 * @brief Function to initialize and control the clock and delays
 *
 * These functions initialize the clock to the maximum allowable frequency
 * of 168 MHz and intialize the millisecond delay 
 *
 */
#include "clock.h"

// Global time delay variable
volatile uint32_t TimeDelay = 0;

/*!
 * @brief Configure the system clock and SysTick
 *
 * This function calls SystemClock_Config and also configures
 * Systick to generate an interrupt every 1 ms.
 *
 * @return 0 on success, 1 on failure
 */
uint8_t initSystemClock(void) {
	
	if (SystemClock_Config()) return 1;

	SysTick_Config(CLOCK_FREQ/1000);

	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
	
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);

	return 0;
}

/*!
 * @brief Delay for an amount of milliseconds
 *
 * @param ms The amount of milliseconds to delay
 */
void delayms(uint16_t ms) {
	TimeDelay = ms;

	while (TimeDelay != 0);
}

/*!
 * @brief Systick interrupt handler
 */
void SysTick_Handler(void) {
	if (TimeDelay > 0) TimeDelay--;
	HAL_IncTick();
}

/*!
 * @brief Configure the system clock
 *
 * This function configures the internal clock to 168 MHz using an 8 MHz
 * external oscillator with the internal PLL. This code was generated by
 * STM32CubeMX, and contains only minor modifications.
 *
 * @return 0 on success
 */
uint8_t SystemClock_Config(void) {
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
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 5; // Clock division (2<=PLLM<=63)
	RCC_OscInitStruct.PLL.PLLN = 210; // Clock multiplication (50<=PLLN<=432)
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2; // PLLP = 2, 4, 6, or 8
	RCC_OscInitStruct.PLL.PLLQ = 7; // 2<=PLLQ<=15 SDIO and RNG clock divider (SDIO=48MHz when PLLQ = 7)
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

	return 0;
}
