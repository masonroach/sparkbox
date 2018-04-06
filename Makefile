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
OBJS = main.o

CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
AS = arm-none-eabi-as
OBJCOPY = arm-none-eabi-objcopy

INCDIRS = -Iinclude/ -I.
LIBS = 

CFLAGS = -mcpu=cortex-m4 -mthumb -O3 -Wall $(INCDIRS)\
	--specs=nosys.specs -DARM_MATH_CM4\

WINDOWS := $(if $(shell grep -E "(Microsoft|WSL)" /proc/version),\
	 "Windows Subsystem",)

.PHONY: all clean flash

all: flash

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(CFLAGS) $(OBJS) $(LIBS)

# Different flashing methods for different systems
flash: $(TARGET)
    # Windows Linux Subsystem
    ifdef WINDOWS
	echo $(WINDOWS)
	/mnt/c/Windows/System32/cmd.exe /C powershell -Command "Copy-Item $(TARGET).bin (Get-WMIObject Win32_Volume | ? {\$$_.Label -q 'DIS_F303VC'} | %{\$$_.DriveLetter})"

    # Linux (RPi probably)
    else
	echo "No OS Detected"
	echo $(WINDOWS)

    endif

clean:
	rm -f *.o *.bin *.map $(TARGET)
