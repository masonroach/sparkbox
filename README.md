### Sparkbox
# Mason Roach
# Patrick Roy

## PINS IN USE CHART
--------------------------------------------------------------------------------
PORT   | 0/8    | 1/9    | 2/10   | 3/11   | 4/12   | 5/13   | 6/14   | 7/15   |
--------------------------------------------------------------------------------
PA0-7  | START  |        |        |        |  DAC   |        |LCD RST |LCD FPS |
--------------------------------------------------------------------------------
PA8-15 |        |AUD STBY|        |        |        | SWDIO  | SWCLK  |        |
--------------------------------------------------------------------------------
PB0-7  |LED ERR |        |        |  SWO   |        |        |        |        |
--------------------------------------------------------------------------------
PB8-15 |        |        |        | MEM WP |BAT SYS |        |        |        |
--------------------------------------------------------------------------------
PC0-7  |  LED0  |  LED1  |  LED2  |  LED3  |  LED4  |  LED5  |  LED6  |  LED7  |
--------------------------------------------------------------------------------
PC8-15 |SDIO D0 |SDIO D1 |SDIO D2 |SDIO D3 |SDIO CLK| SD CD  |        |        |
--------------------------------------------------------------------------------
PD0-7  |FSMC D2 |FSMC D3 |SDIO CMD|        |FSMC NOE|FSMC NWE|F NWAIT |FSMC NE1|
--------------------------------------------------------------------------------
PD8-15 |FSMC D13|FSMC D14|FSMC D15|FSMC CLE|FSMC ALE|FSMC A18|FSMC D0 |FSMC D1 |
--------------------------------------------------------------------------------
PE0-7  |        |        |        |        |        |        |        |FSMC D4 |
--------------------------------------------------------------------------------
PE8-15 |FSMC D5 |FSMC D6 |FSMC D7 |FSMC D8 |FSMC D9 |FSMC D10|FSMC D11|FSMC D12|
--------------------------------------------------------------------------------
PF0-7  |  BT A  |  BT B  |  BT X  |  BT Y  |BT DOWN | BT UP  |BT RIGHT|BT LEFT |
--------------------------------------------------------------------------------
PF8-15 |        |        |        |        |        |        |        |        |
--------------------------------------------------------------------------------
PG0-7  |        |        |        |        |        |        |        |        |
--------------------------------------------------------------------------------
PG8-15 |        |FSM NCE3|        |        |        |        |        |        |
--------------------------------------------------------------------------------

LED[0-7] - Sparkbox LEDs

Start - User button (Sparkbox & Discovery)

LEFT, RIGHT, UP, DOWN, A, B, X, Y - Sparkbox buttons

DAC - Digital to Analog converter

SD card with SDIO
	SDIO D[0-3] - data
	SDIO CLK - clock
	SDIO CMD - command
	SD CD - chip detect

FSMC (Flexible Static Memory Controller) using NAND flash:
	FSMC D[7:0] or fD[15:0] 8 or 16 bit transactions - I/O
	FSMC ALE - address latch enable
	FSMC CLE - command latch enable
	FSMC NCE3 - chip select, x=2,3 - O (fNCE3 for our purposes)
	FSMC NOE - Output enable (memory signal name: read enable, NRE) - O
	FSMC NWE - Write enable - O
	FSMC NWAIT - R/B (ready/busy) input to FSMC - I

## PERIPHERALS/FUNCTIONS IN USE
-----------

SD Driver:
SDIO
EXTI15_10_IRQn
DMA_CHANNEL_4
DMA2_Stream3
DMA2_Stream6

sparkbox push buttons:
EXTI0_IRQn
EXTI1_IRQn
EXTI2_IRQn
EXTI3_IRQn
EXTI4_IRQn
EXTI9_5_IRQn

WAV Player:
DAC1
TIM6
DMA1_Stream5
DMA1_Stream5_IRQn

Video:
FSMC
TIM7
TIM7_IRQn
DMA2_Stream5
DMA2_Stream5_IRQn
