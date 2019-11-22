# Makefile for atmega m328p.
#
# Usage:
#
# make install_fuses	# sets fuses
# make			# compiles the example
# make install		# installs the example

CC=avr-gcc
# compile for atmega328p with warnings, optimizations, and 16 MHz clock frequency
CFLAGS=-Wall -Os -mmcu=atmega328p -DF_CPU=16000000 -std=c99 -save-temps -fverbose-asm
OBJCOPY=avr-objcopy
AVRDUDE=avrdude

DEVICE=/dev/ttyACM0

lcd_st7032_example.hex: lcd_st7032_example
	$(OBJCOPY) -j .text -j .data -O ihex lcd_st7032_example lcd_st7032_example.hex

lcd_st7032_example: lcd_st7032_example.o lcd_st7032.o twimaster.o
	$(CC) $(CFLAGS) -o lcd_st7032_example lcd_st7032_example.o lcd_st7032.o twimaster.o

lcd_st7032_example.o: lcd_st7032_example.c i2cmaster.h lcd_st7032.h
	$(CC) $(CFLAGS) -c lcd_st7032_example.c

lcd_st7032.o: lcd_st7032.c lcd_st7032.h i2cmaster.h
	$(CC) $(CFLAGS) -c lcd_st7032.c

twimaster.o: twimaster.c i2cmaster.h
	$(CC) $(CFLAGS) -c twimaster.c

install_test:
	$(AVRDUDE) -c avrisp -p m328p -P $(DEVICE) -b 19200

install: lcd_st7032_example.hex
	$(AVRDUDE) -c avrisp -p m328p -P $(DEVICE) -b 19200 -U flash:w:lcd_st7032_example.hex:i

install_fuses:
	$(AVRDUDE) -c avrisp -p m328p -P $(DEVICE) -b 19200 -U efuse:w:0xFF:m -U hfuse:w:0xDF:m -U lfuse:w:0xEF:m

clean:
	-rm -f lcd_st7032_example lcd_st7032_example.hex *.o *.s *.i
