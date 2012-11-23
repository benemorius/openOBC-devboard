
ARCH = arm-none-eabi
PROJECT=$(shell pwd | sed -e 's/.*\///')

# Tool definitions
CC      = $(ARCH)-gcc
CPP	= $(ARCH)-g++
LD      = $(ARCH)-ld
AR      = $(ARCH)-ar
AS      = $(ARCH)-as
CP      = $(ARCH)-objcopy
OD	= $(ARCH)-objdump
SIZE	= $(ARCH)-size
GDB	= $(ARCH)-gdb
GDBTUI	= $(ARCH)-gdbtui
RM	= rm

LPCCHECKSUM	= lpcchecksum

INCDIRS = ./
INCDIRS += cmsis/
INCSTRING = $(patsubst %,-I%,$(INCDIRS)) -I.

# Flags
CFLAGS = -W -O0 --std=gnu99 -fgnu89-inline
CFLAGS += -mcpu=cortex-m3  -mthumb -mapcs-frame  -D__thumb2__=1  -msoft-float  -gdwarf-2  -mno-sched-prolog
CFLAGS += -ffunction-sections  -fdata-sections -fno-hosted  -mtune=cortex-m3 -march=armv7-m  -mfix-cortex-m3-ldrd -Wl,-lm
CFLAGS += -Wall
CFLAGS += -Wl,--gc-sections
CFLAGS += -g
CFLAGS += -D__RAM_MODE__=0
CFLAGS += $(INCSTRING)

CPPFLAGS = $(INCSTRING)
CPPFLAGS = $(CFLAGS)

ASFLAGS  = $(INCSTRING)
ASFLAGS += --defsym RAM_MODE=0

LDFLAGS  = -lc -lg -lstdc++ -lsupc++ -lm -lgcc
LDFLAGS += -lcs3 -lcs3unhosted -lcs3micro -pie
LDFLAGS += --gc-sections


CPFLAGS  =

# LINKER_SCRIPT = LPC1768-flash.ld
LINKER_SCRIPT = lpc17xx.ld
# ASRCS  = startup_LPC17xx.s
CSRCS = $(wildcard *.c)
CPPSRCS = $(wildcard *.cpp)
CMSISCSRCS = $(wildcard cmsis/*.c)


OBJS   = $(CSRCS:.c=.o) $(ASRCS:.s=.o) $(CPPSRCS:.cpp=.o)
CMSISOBJS = $(CMSISCSRCS:.c=.o)

.PHONY: all size clean install flash

all: $(PROJECT).elf $(PROJECT).hex

size: $(PROJECT).elf
	@$(SIZE) *.o $<

%.hex: %.elf
	@$(CP) $(CPFLAGS) -O ihex $< $*.hex

%.bin: %.elf
	@$(CP) $(CPFLAGS) -O binary $< $*.bin
	@$(LPCCHECKSUM) $*.bin #write checksum to .bin file; can be commented out

$(PROJECT).elf: $(LINKER_SCRIPT) $(OBJS) cmsis.a
	$(CC) -o $@ $(OBJS) -Xlinker -Map -Xlinker $(PROJECT).map -Xlinker -T $(LINKER_SCRIPT) $(CFLAGS) -l:cmsis.a -lstdc++
	@$(SIZE) *.o $@


cmsis.a: $(CMSISOBJS)
	@$(AR) r $@ $(CMSISOBJS)

%.o: %.c
	$(CC) $(CFLAGS) -MM $< -MP
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

%.o: %.cpp
	$(CPP) $(CPPFLAGS) -MM $< -MP
	$(CPP) $(CPPFLAGS) -c $< -o $@

clean:
	@rm -f *.hex *.bin
	@-rm -f *.elf quiet.log
	@-\
for D in "." "**"; do \
  rm -f $$D/*.o $$D/*.d $$D/*.lst $$D/*.dump $$D/*.map $$D/*.a; \
done

flash: $(PROJECT).bin
	flash $(PROJECT).bin

install: flash

debug: $(PROJECT).elf
	$(GDBTUI) -x openocd.gdb
