
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
INCDIRS += lib/fatfs lib/SDFS
INCDIRS += lib/AnalogIn
INCDIRS += lib/E36Diag lib/DS2
INCDIRS += lib/ConfigFile lib/Debug lib/FunctionPointer lib/IO lib/Input lib/InterruptManager lib/PWM lib/RTC lib/SPI lib/Stream lib/Timer lib/Uart lib/Callback lib/Watchdog
INCDIRS += lib/PCA95xx lib/I2C
INCDIRS += lib/MMA845x
INCDIRS += lib/AnalogOut
INCDIRS += tasks/ObcCode
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
CFLAGS += -DGIT_VERSION="\"`git describe --always --dirty --abbrev=6`\""

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
CSRCS = $(wildcard *.c) $(wildcard lib/fatfs/*.c) $(wildcard lib/fatfs/option/*.c)
CPPSRCS = $(wildcard *.cpp) $(wildcard lib/SDFS/*.cpp) $(wildcard lib/fatfs/*.cpp)
CPPSRCS += $(wildcard lib/AnalogIn/*.cpp)
CPPSRCS += $(wildcard lib/E36Diag/*.cpp) $(wildcard lib/DS2/*.cpp)
CPPSRCS += $(wildcard lib/ConfigFile/*.cpp) $(wildcard lib/Debug/*.cpp) $(wildcard lib/FunctionPointer/*.cpp) $(wildcard lib/IO/*.cpp) $(wildcard lib/Input/*.cpp) $(wildcard lib/InterruptManager/*.cpp) $(wildcard lib/PWM/*.cpp) $(wildcard lib/RTC/*.cpp) $(wildcard lib/SPI/*.cpp) $(wildcard lib/Stream/*.cpp) $(wildcard lib/Timer/*.cpp) $(wildcard lib/Uart/*.cpp) $(wildcard lib/Callback/*.cpp) $(wildcard lib/Watchdog/*.cpp)
CPPSRCS += $(wildcard lib/PCA95xx/*.cpp) $(wildcard lib/I2C/*.cpp)
CPPSRCS += $(wildcard lib/MMA845x/*.cpp)
CPPSRCS += $(wildcard lib/AnalogOut/*.cpp)
CPPSRCS += $(wildcard tasks/ObcCode/*.cpp)
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
	@-rm -f *.elf
	@-\
for D in "." "cmsis" "lib/SDFS" "lib/fatfs" "lib/fatfs/option" "lib/AnalogIn" "lib/E36Diag" "lib/DS2" "lib/ConfigFile" "lib/Debug" "lib/FunctionPointer" "lib/IO" "lib/Input" "lib/InterruptManager" "lib/PWM" "lib/RTC" "lib/SPI" "lib/Stream" "lib/Timer" "lib/Uart" "lib/Callback" "lib/Watchdog" "lib/PCA95xx" "lib/I2C" "lib/MMA845x" "lib/AnalogOut" "tasks/ObcCode"; do \
  rm -f $$D/*.o $$D/*.d $$D/*.lst $$D/*.dump $$D/*.map $$D/*.a; \
done

flash: $(PROJECT).bin
	flash $(PROJECT).bin

install: flash

debug: $(PROJECT).elf
	$(GDBTUI) -x openocd.gdb
