#ifndef SPARKBOX_PUSHBUTTONS
#define SPARKBOX_PUSHBUTTONS

#include <stdlib.h>
#include <stdbool.h>
#include "stm32f4xx.h"


extern volatile bool BUTTON_LEFT; 	// PA1
extern volatile bool BUTTON_RIGHT; // PA2
extern volatile bool BUTTON_UP;		// PA3
extern volatile bool BUTTON_DOWN;	// PA4
extern volatile bool BUTTON_A;			// PA5
extern volatile bool BUTTON_B;			// PA6
extern volatile bool BUTTON_X;			// PA7
extern volatile bool BUTTON_Y;			// PA8

void initButtons(void);

#endif