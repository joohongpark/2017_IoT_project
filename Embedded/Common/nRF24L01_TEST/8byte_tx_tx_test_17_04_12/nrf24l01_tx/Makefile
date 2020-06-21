DEVICE     = atmega128
CLOCK      = 16000000
PROGRAMMER = -c stk500v2 -P /dev/tty.SLAB_USBtoUART
OBJECTS    = main.o
AVRDUDE = avrdude $(PROGRAMMER) -p $(DEVICE)
COMPILE = avr-gcc -Wall -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE)
TARGET = main

# symbolic targets:
all:	main.hex

.c.o:
	@echo 컴파일
	$(COMPILE) -c $(TARGET).c -o $(OBJECTS)

clean:
	@echo 클린
	rm -f main.hex main.elf $(OBJECTS)

flash:	all
	$(AVRDUDE) -U flash:w:main.hex:i
# file targets:
main.elf: $(OBJECTS)
	$(COMPILE) -o main.elf $(OBJECTS)

main.hex: main.elf
	rm -f main.hex
	avr-objcopy  -O ihex $< $@
	avr-size --format=avr --mcu=$(DEVICE) main.elf
# If you have an EEPROM section, you must also create a hex file for the
# EEPROM and add it to the "flash" target.

# Targets for code debugging and analysis:
disasm:	main.elf
	avr-objdump -d main.elf

cpp:
	$(COMPILE) -E main.c
