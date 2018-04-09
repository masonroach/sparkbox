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

TARGET = exec

PREFIX = arm-none-eabi-
CC = $(PREFIX)gcc
LD = $(PREFIX)ld
AS = $(PREFIX)as
OBJCOPY = $(PREFIX)objcopy

# Optimization
# O3 -> Lots of optimization
# Os -> Optimize size
# Og -> Optimize for debugging
OPTIMIZE = -O3

# Directories
SRCDIR    := src
INCDIR    := inc
OBJDIR    := obj
LIBDIR    := lib
TARGETDIR := bin

# Find source files and declare objects
CSRC  := $(shell find $(SRCDIR) -type f -name *.c)
SSRC  := $(shell find $(SRCDIR) -type f -name *.s)
LDSRC  = $(LIBDIR)/STM32F303VCTx_FLASH.ld
OBJS   = $(patsubst $(SRCDIR)/%, $(OBJDIR)/%, $(CSRC:.c=.o))
OBJS  += $(patsubst $(SRCDIR)/%, $(OBJDIR)/%, $(SSRC:.s=.o))

# Define vpaths
vpath %.c  $(SRCDIR)
vpath %.h  $(INCDIR)
vpath %.o  $(OBJDIR)
vpath %.s  $(SRCDIR)
vpath %.ld $(LIBDIR)

INCDIRS = -I$(INCDIR) -I.
LIBS = 

# CPU defines
CPU = -mcpu=cortex-m4
MCFLAGS = $(CPU) -mthumb

CFLAGS = $(MFLAGS) $(OPTIMIZE) -Wall $(INCDIRS) -fdata-sections\
	--specs=nosys.specs -DARM_MATH_CM4 -ffunction-sections\
	-T $(LDSRC)\

# Find if running on a windows subsystem
WINDOWS := $(if $(shell grep -E "(Microsoft|WSL)" /proc/version),\
	 "Windows Subsystem",)

.PHONY: all clean flash

t = $(shell echo $(OBJS))

all: flash

$(TARGET): $(OBJS)
	$(CC) -o $@ $(CFLAGS) $^ $(LIBS)

$(TARGET).bin: $(TARGET)
	$(OBJCOPY) -Obinary $< $@

# Different flashing methods for different systems
flash: $(TARGET).bin
    # Windows Linux Subsystem
    ifdef WINDOWS
	/mnt/c/Windows/System32/cmd.exe /C powershell -Command "Copy-Item $(TARGET).bin (Get-WMIObject Win32_Volume | ? {\$$_.Label -eq 'DIS_F303VC'} | %{\$$_.DriveLetter})"

    # Linux (RPi probably)
    else
	echo "No OS Detected. No flash rule provided. . . "

    endif

clean:
	rm -f $(OBJDIR)/*.o *.bin *.map $(TARGET)

# Compile c objects rule
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

# Compile s objects rule
$(OBJDIR)/%.o: $(SRCDIR)/%.s
	@mkdir -p $(@D)
	$(AS) -c $(MCFLAGS) -o $@ $<
