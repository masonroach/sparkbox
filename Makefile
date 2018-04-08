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

CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
AS = arm-none-eabi-as
OBJCOPY = arm-none-eabi-objcopy

# Directories
SRCDIR    := src
INCDIR    := include
OBJDIR    := obj
TARGETDIR := bin

# Define vpaths
vpath %.c $(SRCDIR)
vpath %.h $(INCDIR)
vpath %.o $(OBJDIR)

INCDIRS = -I$(INCDIR) -I.
LIBS = 

CFLAGS = -mcpu=cortex-m4 -mthumb -O3 -Wall $(INCDIRS)\
	--specs=nosys.specs -DARM_MATH_CM4\

# Find if running on a windows subsystem
WINDOWS := $(if $(shell grep -E "(Microsoft|WSL)" /proc/version),\
	 "Windows Subsystem",)

# Find source files and declare objects
SOURCES := $(shell find $(SRCDIR) -type f -name *.c)
OBJECTS := $(patsubst $(SRCDIR)/%, $(OBJDIR)/%, $(SOURCES:.c=.o))

.PHONY: all clean flash

all: flash

$(TARGET): $(OBJECTS)
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

# Compile objects rule
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<
