#ifndef SPARKBOX_PUSHBUTTONS
#define SPARKBOX_PUSHBUTTONS

#include <stdlib.h>
#include <stdbool.h>
#include "stm32f4xx.h"

extern volatile uint8_t buttons;

#define ALL_BUTTONS (buttons)

#define BUTTON_A 		((buttons >> 0) & 0x01)
#define BUTTON_B 		((buttons >> 1) & 0x01)
#define BUTTON_X 		((buttons >> 2) & 0x01)
#define BUTTON_Y 		((buttons >> 3) & 0x01)
#define BUTTON_DOWN 	((buttons >> 4) & 0x01)
#define BUTTON_UP 		((buttons >> 5) & 0x01)
#define BUTTON_RIGHT 	((buttons >> 6) & 0x01)
#define BUTTON_LEFT 	((buttons >> 7) & 0x01)

#define PUSHED 0x01
#define RELEASED 0x00

void initButtons(void);

#endif
