#### Sparkbox

## Patch Notes
* 6/12/2018 - Updated all of the device files to run on F4. Buttton.[ch] and led.[ch] have been updated and are functioning properly on the F4 discovery board.
* 6/13/2018 - Updated USART files. They compile properly, but have not been tested yet.

## PINS IN USE
-----------
SPI 1: 
PB2 - CS
PB5 - MOSI
PB4 - MISO
PB3 - SCK

Push Buttons (for Sparkbox):
PD8 - 15

LEDs (on Discovery Board):
PD12 - 15

Push button (on Discovery Board):
PA0

UART1:
PA9 - TX
PA10 - RX

PWM (Audio):
PB6 - TIM4_CH1
PB7 - TIM4_CH2
PB8 - TIM4_CH3
PB9 - TIM4_CH4
