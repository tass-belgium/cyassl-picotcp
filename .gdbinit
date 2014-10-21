 tar ext :3333
 monitor reset init
 set remotetimeout 100
 file out/main.elf
 load out/main.elf
 mon reset halt
 stepi
 focus c
