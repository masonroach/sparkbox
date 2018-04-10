# Sparkbox
#
# Mason Roach
# Patrick Roy
#
# ============================ Instructions ============================
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
# ======================================================================

TARGET = main

# Define compilers
PREFIX = arm-none-eabi
CC = $(PREFIX)-gcc
LD = $(PREFIX)-gcc
AS = $(PREFIX)-as
CP = $(PREFIX)-objcopy
OD = $(PREFIX)-objdump

# Optimization
# O3 -> Lots of optimization
# Os -> Optimize size
# Og -> Optimize for debugging
OPTIMIZE = -Os

# Directories
SRCDIR    := src
INCDIR    := inc
OBJDIR    := obj
LIBDIR    := lib
TARGETDIR := bin

# Define vpaths
vpath %.c  $(SRCDIR)
vpath %.h  $(INCDIR)
vpath %.o  $(OBJDIR)
vpath %.s  $(SRCDIR)
vpath %.ld $(LIBDIR)

# Find source files and declare objects
SRC   := $(shell find $(SRCDIR) -type f -name *.c)
OBJS   = $(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(SRC:.c=.o))

STARTUP = $(LIBDIR)/startup_stm32f303xc.s

LINKER  = $(LIBDIR)/STM32F303VCTx_FLASH.ld

# CPU defines
MCU = cortex-m4
MCFLAGS = -mcpu=$(MCU) -mthumb -mthumb-interwork

INCLUDES = -I$(INCDIR) -I.

# Define compiler flags
CFLAGS = $(MCFLAGS) $(OPTIMIZE) $(INCLUDES) -Wl,-T,$(LINKER) \
	-lnosys

ASFLAGS = $(MCFLAGS)

# Find if running on a windows subsystem
WINDOWS := $(if $(shell grep -E "(Microsoft|WSL)" /proc/version),\
	 "Windows Subsystem",)

# ======================================================================
#   Compilation
# ======================================================================

all: flash

# Objcopy for binary
$(TARGET).bin: $(TARGET)
	$(CP) -O binary $< $@

# Objcopy for hex
$(TARGET).hex: $(TARGET)
	$(CP) -O ihex $^ $@

# Compile executable
$(TARGET): $(OBJS) $(STARTUP)
	echo $(SRC)
	$(CC) $(CFLAGS) $^ -o $@

# Compile c objects rule
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

# ======================================================================
#   Other Commands
# ======================================================================

# Remove compiled executables
clean:
	rm -f $(TARGET) $(TARGET).hex $(TARGET).bin $(OBJS)

# Different flashing methods for different systems
flash: $(TARGET).bin
    # Windows Linux Subsystem
    ifdef WINDOWS
	/mnt/c/Windows/System32/cmd.exe /C powershell -Command "Copy-Item $(TARGET).bin (Get-WMIObject Win32_Volume | ? {\$$_.Label -eq 'DIS_F303VC'} | %{\$$_.DriveLetter})"

    # Linux (RPi probably)
    else
	echo "No OS Detected. No flash rule provided. . . "

    endif
