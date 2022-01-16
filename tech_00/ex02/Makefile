FILENAME	= main
# PORT		= /dev/cu.SLAB_USBtoUART
PORT		= /dev/ttyUSB0
DEVICE		= atmega328p
PROGRAMMER	= arduino
BAUD		= 115200
F_CPU		= 16000000 #16 Mhz quartz #2 Mhz Cpu Clock (Quartz / 8)
COMPILE		= avr-gcc -Wall -mmcu=$(DEVICE) -DF_CPU=${F_CPU}

all: hex flash

monitor: all
	@screen $(PORT) $(BAUD)

hex:
	$(COMPILE) -c $(FILENAME).c -o $(FILENAME).o
	$(COMPILE) -o $(FILENAME).bin $(FILENAME).o
	avr-objcopy -O ihex $(FILENAME).bin $(FILENAME).hex
	avr-size --format=avr --mcu=$(DEVICE) $(FILENAME).bin

flash:
	avrdude -v -p $(DEVICE) -c $(PROGRAMMER) -P $(PORT) -b $(BAUD) -U flash:w:$(FILENAME).hex:i
	@echo "Msg dans eeprom"

	# avrdude -v -p $(DEVICE) -c $(PROGRAMMER) -P $(PORT) -b $(BAUD) -U eeprom:w:eeprom.hex:r
	avrdude -v -p $(DEVICE) -c $(PROGRAMMER) -P $(PORT)  -U eeprom:w:0xff:m

clean:
	rm -f $(FILENAME).o
	rm -f $(FILENAME).bin
	rm -f $(FILENAME).hex