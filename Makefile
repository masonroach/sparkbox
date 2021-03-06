# Sparkbox
#
# Mason Roach
# Patrick Roy
#
# ================================ Instructions ================================
#
# [make]
# 	Compile the binary file and flash it to the STM Discovery board
#
# [make clean]
#	Remove compiled or unessecary files, such as object files,
#	binary files, and executables
#
# [make flash]
#	Flashes the STM board with the binary file. If the file has not
#	yet been compiled, it is comiled first. Flashing methods are
#	handled dependant on operating system
#
# ==============================================================================

# Define compilers
PREFIX = arm-none-eabi
CC = $(PREFIX)-gcc
LD = $(PREFIX)-ld
AS = $(PREFIX)-as
CP = $(PREFIX)-objcopy
OD = $(PREFIX)-objdump

# Optimization
# O3 -> Lots of optimization
# Os -> Optimize size
# Og -> Optimize for debugging
OPTIMIZE = -Os

# Directories
SRCDIR           := src
INCDIR           := inc
OBJDIR           := obj
LIBDIR           := lib
  SYSDIR         := $(LIBDIR)/system
    SYSSRCDIR    := $(SYSDIR)/src
    SYSINCDIR    := $(SYSDIR)/inc
  FATDIR         := $(LIBDIR)/FatFs/src
    FATOPTION    := $(FATDIR)/option
  HALDIR	 := $(LIBDIR)/STM32F4xx_HAL_Driver
    HALINCDIR    := $(HALDIR)/Inc
    HALSRCDIR    := $(HALDIR)/Src
  USDDIR	 := $(LIBDIR)/FatFs_uSD
    USDINCDIR    := $(USDDIR)/Inc
    USDSRCDIR    := $(USDDIR)/Src
TARGETDIR        := bin

# Define vpaths
vpath %.c  $(SRCDIR):$(SYSSRCDIR):$(FATDIR):$(FATOPTION):$(FATDRIVER):$(HALSRCDIR):$(USDSRCDIR)
vpath %.o  $(OBJDIR)
vpath %.s  $(SYSDIR)
vpath %.ld $(LIBDIR)

# Define target
TARGET = $(TARGETDIR)/main

STARTUP = $(SYSDIR)/startup_stm32f407xx.s

# Find source files and declare objects. Does not include hidden files.
SRC   := $(shell find $(SRCDIR) -type f -name [^.]*.c)
SRC   += $(shell find $(SYSSRCDIR) -type f -name [^.]*.c)
SRC   += $(shell find $(FATDIR) -type f -name [^.]*.c)
SRC   += $(shell find $(HALDIR) -type f -name [^.]*.c)
SRC   += $(shell find $(USDDIR) -type f -name [^.]*.c)
OBJS  := $(addprefix $(OBJDIR)/,$(notdir $(SRC:.c=.o)))
OBJS  += $(addprefix $(OBJDIR)/,$(notdir $(STARTUP:.s=.o)))

LINKER  = $(LIBDIR)/stm32f407vgtx_flash.ld

# CPU defines
MCU = cortex-m4
MCFLAGS = -mcpu=$(MCU) -mthumb -mthumb-interwork

# Define include paths
INCLUDES := $(INCDIR) $(SYSINCDIR) $(FATDIR) $(FATOPTION) $(FATDRIVER) $(HALINCDIR) $(USDINCDIR)
INCFLAGS := $(addprefix -I, $(INCLUDES))

# Define compiler flags
CFLAGS = $(MCFLAGS) $(OPTIMIZE) $(INCFLAGS) -Wall -Wl,-T,$(LINKER) \
	-lnosys -lc -lm -lgcc

ASFLAGS = $(MCFLAGS)

# Find if running on a windows subsystem
WINDOWS := $(if $(shell grep -E "(Microsoft|WSL)" /proc/version),\
	 "Windows Subsystem",)
LINUX   := $(if $(shell grep -E "Ubuntu" /proc/version), "Ubuntu")

# ==============================================================================
#   Compilation
# ==============================================================================

all: flash

# Objcopy for binary
$(TARGET).bin: $(TARGET)
	$(CP) -O binary $< $@

# Objcopy for hex
$(TARGET).hex: $(TARGET)
	$(CP) -O ihex $^ $@

# Compile executable
$(TARGET): $(OBJS)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) --specs=nosys.specs $^ -o $@

# Compile c objects rule
$(OBJDIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

# Compile s objects rule
$(OBJDIR)/%.o: %.s
	@mkdir -p $(@D)
	$(AS) $(ASFLAGS) -o $@ $<

# ==============================================================================
#   Other Commands
# ==============================================================================

# Remove compiled executables
clean:
	rm -f $(TARGET) $(TARGET).hex $(TARGET).bin $(OBJDIR)/*.o

# Different flashing methods for different systems
flash: $(TARGET).bin
  # Windows Linux Subsystem
  ifdef WINDOWS
	/mnt/c/Windows/System32/cmd.exe /C powershell -Command \
	"Copy-Item $< (Get-WMIObject Win32_Volume \
	| ? {\$$_.Label -eq 'DIS_F407VG'} | %{\$$_.DriveLetter})"

  # Linux (RPi probably)
  else
  ifdef LINUX
	st-flash write bin/main.bin 0x8000000
  endif
  endif
