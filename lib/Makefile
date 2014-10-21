.PHONY: all

PREFIX?=$(PWD)/../out

all: $(PREFIX)/lib/libhttps.a

$(PREFIX)/lib/libcyassl.a:
	@cp Makefile.cyassl cyassl/Makefile
	@$(MAKE) -C cyassl/ PREFIX=$(PREFIX)

$(PREFIX)/lib/libhttps.a: $(PREFIX)/lib/libpicotcp.a  $(PREFIX)/lib/libcyassl.a $(PREFIX)/lib/bsp.a
	@cp Makefile.libhttps picotcp-modules/libhttps/Makefile
	@$(MAKE) -C picotcp-modules/libhttps PREFIX=$(PREFIX)

$(PREFIX)/lib/libpicotcp.a: 
	@$(MAKE) -C picotcp/ ARCH=stm32f4xx CROSS_COMPILE=arm-none-eabi- PREFIX=$(PREFIX)

$(PREFIX)/lib/bsp.a:
	@$(MAKE) -C stm32f4xx/ PREFIX=$(PREFIX)
