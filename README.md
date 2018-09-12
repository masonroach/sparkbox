#### Sparkbox

## Patch Notes
* 6/12/2018 - Updated all of the device files to run on F4. Buttton.[ch] and led.[ch] have been updated and are functioning properly on the F4 discovery board.
* 6/13/2018 - Updated USART files. They compile properly, but have not been tested yet.

## PINS
-----------
SPI 1: (Not being used at the moment)
PB2 - CS
PB5 - MOSI
PB4 - MISO
PB3 - SCK

Discovery LEDs:
PD12-15


## PINS IN USE CHART
-------------------------------------------------------------------------
PORT	| 0/8	| 1/9	| 2/10	| 3/11	| 4/12	| 5/13	| 6/14	| 7/15	|
-------------------------------------------------------------------------
PA0-7	| USER	| LEFT	| RIGHT	| UP	| DOWN	| DAC	| B	| X	|
-------------------------------------------------------------------------
PA8-15	| Y	| TX	| RX	| 	| 	| 	| 	| 	|
-------------------------------------------------------------------------
PB0-7	|LCD RD |LCD WR |SPI CS |SPI SCK|SPIMISO|SPIMOSI|LCD DC |LCD CS |
-------------------------------------------------------------------------
PB8-15	|LCD RST| 	| 	| 	| 	| 	| 	| 	|
-------------------------------------------------------------------------
PC0-7	| sLED0	| sLED1	| sLED2	| sLED3	| sLED4	| sLED5	| sLED6	| sLED7	|
-------------------------------------------------------------------------
PC8-15	| sdD0	| sdD1	| sdD2	| sdD3	| sdCLK	| sdCD	| 	| 	|
-------------------------------------------------------------------------
PD0-7	| fD2	| fD3	| sdCMD	| 	| fNOE	| fNWE	| fNWAIT| fNCE2	|
-------------------------------------------------------------------------
PD8-15	| fD13	| fD14	| fD15	| fA16	| fA17	| 	| fD0	| fD1	|
-------------------------------------------------------------------------
PE0-7	| 	| 	| 	| 	| 	| 	| 	| fD4	|
-------------------------------------------------------------------------
PE8-15	| fD5	| fD6	| fD7	| fD8	| fD9	| fD10	| fD11	| fD12	|
-------------------------------------------------------------------------
PF0-7	| 	| 	| 	| 	| 	| 	| 	| 	|
-------------------------------------------------------------------------
PF8-15	| 	| 	| 	| 	| 	| 	| 	| 	|
-------------------------------------------------------------------------
PG0-7	| 	| 	| 	| 	| 	| 	| fINT2	| fINT3	|
-------------------------------------------------------------------------
PG8-15	| 	| fNCE3	| 	| 	| 	| 	| 	| 	|
-------------------------------------------------------------------------

sLED[0-7] - Sparkbox LEDs

USER - User button (Sparkbox & Discovery)

TX/RX - UART1

LEFT, RIGHT, UP, DOWN, A, B, X, Y - Sparkbox buttons

DAC - Digital to Analog converter

SD card with SDIO
	sdD[0-3] - data
	sdCLK - clock
	sdCMD - command
	sdCD - chip detect

FSMC (Flexible Static Memory Controller) using NAND flash:
	fD[7:0] or fD[15:0] 8 or 16 bit transactions - I/O
	fNCE[x] chip select, x=2,3 - O
	fNOE or NRE - Output enable (memory signal name: read enable, NRE) - O
	fNWE - Write enable - O
	fNWAIT/fINT[3:2] - ready/busy input to FSMC - I

## PERIPHERALS/FUNCTIONS IN USE
-----------

SD Driver:
SDIO
EXTI15_10_IRQn
DMA_CHANNEL_4
DMA2_Stream3
DMA2_Stream6

sparkbox push buttons:
EXTI1_IRQn
EXTI2_IRQn
EXTI3_IRQn
EXTI4_IRQn
EXTI9_5_IRQn
