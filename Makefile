TCPREFIX = arm-none-eabi-
CC      = $(TCPREFIX)gcc
AS      = $(TCPREFIX)gcc
LD      = $(TCPREFIX)gcc
CP      = $(TCPREFIX)objcopy
OD      = $(TCPREFIX)objdump
GDBTUI  = $(TCPREFIX)gdb

STM32FLASH = ./stm32_flash.pl

ASM_FILES := $(wildcard src/*.s)
CXX_FILES := $(wildcard src/*.c)
OBJ_FILES := $(addprefix obj/,$(notdir $(CXX_FILES:.c=.o)))
OBJ_FILES += $(addprefix obj/,$(notdir $(ASM_FILES:.s=.o)))
INC_DIRS := out/include
INC_DIRS := $(addprefix -I,$(INC_DIRS))
OUT_FILE := out/main
LD_FILE := stm32.ld

CFLAGS  =  -I. $(INC_DIRS) -O2 -g -Wall -DSTM32F407xx -DSTM32
CFLAGS += -mcpu=cortex-m4 -mthumb -mlittle-endian -mthumb-interwork
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16 
LFLAGS  =  $(CFLAGS) -T$(LD_FILE) -nostartfiles -lc -lnosys -DSTM32 -specs=nosys.specs  -lm
CPFLAGS = -Obinary
ODFLAGS = -S

PREFIX:=$(PWD)/out
################################################################
## Colordefinition
################################################################
NO_COLOR        = \x1b[0m
GREEN_COLOR     = \x1b[32;01m
YELLOW_COLOR    = \x1b[33;01m
RED_COLOR       = \x1b[31;01m
WHITE_COLOR     = \x1b[37;01m

.PHONY: all clean

all: $(OUT_FILE).bin 

clean:
	rm -rf out 
	find . -name "*.o" | xargs rm

flash: $(OUT_FILE).bin
	$(STM32FLASH) $^

$(OUT_FILE).bin: $(OUT_FILE).elf
	$(CP) $(CPFLAGS) $(OUT_FILE).elf $(OUT_FILE).bin
	$(CP) $(CPFLAGS) $(OUT_FILE).elf -O srec --srec-len 19 $(OUT_FILE).s19
	$(CP) $(CPFLAGS) $(OUT_FILE).elf -O ihex $(OUT_FILE).hex

$(OUT_FILE).elf: out/lib $(OBJ_FILES)
	$(LD) $(CFLAGS) -Wl,--verbose -o $@ -Iout/include -Wl,--start-group out/lib/lib* $(OBJ_FILES) -Wl,--end-group -Wl,-Map,$(OUT_FILE).map $(LFLAGS)

out/lib:
	$(MAKE) -C lib/ PREFIX=$(PREFIX) 
	

obj/%.o: src/%.c 
	$(CC) -c $(CFLAGS) $< -o $@

obj/%.o: src/%.s
	$(AS) -c $(CFLAGS) $< -o $@

debug: $(OUT_FILE).elf
	$(GDBTUI) $^
