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

SDIO (In order of microSD connection):
PC10 - D2
PC11 - D3
PD2  - CMD
3.3V - VDD
PC12 - CLK
GND  - VSS
PC8  - D0
PC9  - D1

Push Buttons (on Sparkbox):
PB8 - 15

LEDs (on Sparkbox):
PC0 - 7
PA0

LEDs (on Discovery Board):
PD12 - 15

Push button (on Discovery Board):
PA0

UART1:
PA9 - TX
PA10 - RX

DAC (Audio)
PA4 - DAC_OUT1
PA5 - DAC_OUT2 (likely not used)

FSMC (Flexible Static Memory Controller)
Using NAND flash
	PD12 - A[17] - address latch enable (ALE) - O
	PD11 - A[16] - command latch enable (CLE) - O
D[7:0] or D[15:0] 8 or 16 bit transactions - I/O
	PD14 - D0
	PD15 - D1
	PD0  - D2
	PD1  - D3
	PE7  - D4
	PE8  - D5
	PE9  - D6
	PE10 - D7
	PE11 - D8
	PE12 - D9
	PE13 - D10
	PE14 - D11
	PE15 - D12
	PD8  - D13
	PD9  - D14
	PD10 - D15
NCE[x] chip select, x=2,3 - O
	PD7 - NCE2
	PG9 - NCE3
NOE or NRE - Output enable (memory signal name: read enable, NRE) - O
	PD4 - NOE
NWE - Write enable - O
	PD5 - NWE
NWAIT/INT[3:2] - ready/busy input to FSMC - I
	PD6 - NWAIT
	PG6 - INT2
	PG7 - INT3
