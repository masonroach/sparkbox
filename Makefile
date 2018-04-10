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

MCU = cortex-m4
MCFLAGS = -mcpu=$(MCU) -mthumb -mlittle-endian -mthumb-interwork

INCLUDES = -I$(INCDIR) -I.

CFLAGS = $(MCFLAGS) $(OPTIMIZE) $(INCLUDES) -Wl,-T,$(LINKER) \
	-DDEBUG -lnosys

ASFLAGS = $(MCFLAGS)

# ======================================================================
#   Compilation
# ======================================================================

all: $(TARGET).bin $(OBJS)

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

# Remove compiled executables
clean:
	rm -f $(TARGET) $(TARGET).hex $(TARGET).bin

# Remove objects too
reallyclean: clean
	rm -f $(OBJS)

# ======================================================================
#   Rules
# ======================================================================

# Compile c objects rule
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<
