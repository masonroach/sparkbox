#ifndef SPARK_BUTTON
#define SPARK_BUTTON
#include "stm32f4xx.h"

extern volatile uint8_t buttons;

#define WAIT_UNTIL_PUSH(__BUTTON_) 	while(__BUTTON__ == RELEASED)
#define WAIT_UNTIL_RELEASE(__BUTTON__) while(__BUTTON__ == PUSHED)

#define BUTTON_A        ((buttons >> 0) & 0x01)
#define BUTTON_B        ((buttons >> 1) & 0x01)
#define BUTTON_X        ((buttons >> 2) & 0x01)
#define BUTTON_Y        ((buttons >> 3) & 0x01)
#define BUTTON_DOWN     ((buttons >> 4) & 0x01)
#define BUTTON_UP       ((buttons >> 5) & 0x01)
#define BUTTON_RIGHT    ((buttons >> 6) & 0x01)
#define BUTTON_LEFT     ((buttons >> 7) & 0x01)

#define PUSHED 0x01
#define RELEASED 0x00


void initButtons(void);
uint8_t readButton(void);

#endif
