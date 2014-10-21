PREFIX?=$(PWD)/../../out
CC:=gcc
AR?=ar
RANLIB?=ranlib
CROSS_COMPILE:=arm-none-eabi-
OBJS:=  \
system_stm32f4xx.o \
stm32f4xx_hal_adc.o  \
stm32f4xx_hal_adc_ex.o \
stm32f4xx_hal.o \
stm32f4xx_hal_can.o \
stm32f4xx_hal_cortex.o \
stm32f4xx_hal_crc.o \
stm32f4xx_hal_cryp.o \
stm32f4xx_hal_cryp_ex.o \
stm32f4xx_hal_dac.o \
stm32f4xx_hal_dac_ex.o \
stm32f4xx_hal_dcmi.o \
stm32f4xx_hal_dma2d.o \
stm32f4xx_hal_dma.o \
stm32f4xx_hal_dma_ex.o \
stm32f4xx_hal_eth.o \
stm32f4xx_hal_flash.o \
stm32f4xx_hal_flash_ex.o \
stm32f4xx_hal_gpio.o \
stm32f4xx_hal_hash.o \
stm32f4xx_hal_hash_ex.o \
stm32f4xx_hal_hcd.o \
stm32f4xx_hal_i2c.o \
stm32f4xx_hal_i2c_ex.o \
stm32f4xx_hal_i2s.o \
stm32f4xx_hal_i2s_ex.o \
stm32f4xx_hal_irda.o \
stm32f4xx_hal_iwdg.o \
stm32f4xx_hal_ltdc.o \
stm32f4xx_hal_nand.o \
stm32f4xx_hal_nor.o \
stm32f4xx_hal_pccard.o \
stm32f4xx_hal_pcd.o \
stm32f4xx_hal_pwr.o \
stm32f4xx_hal_pwr_ex.o \
stm32f4xx_hal_rcc.o \
stm32f4xx_hal_rcc_ex.o \
stm32f4xx_hal_rng.o \
stm32f4xx_hal_rtc.o \
stm32f4xx_hal_rtc_ex.o \
stm32f4xx_hal_sai.o \
stm32f4xx_hal_sd.o \
stm32f4xx_hal_sdram.o \
stm32f4xx_hal_smartcard.o \
stm32f4xx_hal_spi.o \
stm32f4xx_hal_sram.o \
stm32f4xx_hal_tim.o \
stm32f4xx_hal_tim_ex.o \
stm32f4xx_hal_uart.o \
stm32f4xx_hal_usart.o \
stm32f4xx_hal_wwdg.o \
stm32f4xx_ll_fmc.o \
stm32f4xx_ll_fsmc.o \
stm32f4xx_ll_sdmmc.o \
stm32f4xx_ll_usb.o \
hardfault_debug.o

#stm32f4xx_hal_msp_template.o 

CFLAGS  =  -I. $(INC_DIRS) -O2 -g -Wall
CFLAGS += -mcpu=cortex-m4 -mthumb -mlittle-endian -mthumb-interwork
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS+= -Iconf/default
EXTRA_INC=conf/default

ARCH?=stm32
CFLAGS+=-IInc -Iconf $(EXTRA_CFLAGS) $(PLATFORM_CFLAGS) -I$(PREFIX)/include -Icmsis/inc -Icmsis/devinc -DSTM32F407xx

all: $(PREFIX)/lib/libbsp.a

%.o: Src/%.c
	$(CROSS_COMPILE)$(CC) -c $(CFLAGS) -o $@ $<

$(PREFIX)/lib/libbsp.a: $(OBJS)
	@cp Inc/* $(PREFIX)/include
	@cp $(EXTRA_INC)/* $(PREFIX)/include
	@cp cmsis/inc/* $(PREFIX)/include
	@cp cmsis/devinc/* $(PREFIX)/include
	@mkdir -p $(PREFIX)/obj
	@mkdir -p $(PREFIX)/lib
	@cp $(OBJS) $(PREFIX)/obj
	@$(CROSS_COMPILE)ar cru $@ $(OBJS)
	@$(CROSS_COMPILE)ranlib $@
	

clean:
	rm -f *.o *.a
