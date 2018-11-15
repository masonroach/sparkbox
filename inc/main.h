#ifndef MAINH
#define MAINH

/* Include architecture register definitions */
#include "stm32f4xx.h"
#include "core_cm4.h"

/* Include HAL for SD card library */
#include "stm32f4xx_hal.h"

/* FatFs includes component */
#include "ff_gen_drv.h"
#include "sd_diskio.h"

#include "clock.h"	// Include clock before others
// #include "usart.h"
#include "led.h"
#include "button.h"
#include "lcd.h"
// #include "rng.h"
// #include "sd.h"
// #include "pwm.h"
#include "sparkboxButtons.h"
#include "waveplayer.h"
#include "sprite.h"
//#include "video.h"

#endif
