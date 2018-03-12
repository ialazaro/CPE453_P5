#Compile the code
main: program2.c serial.c os.c 
	avr-gcc -mmcu=atmega2560 -DF_CPU=16000000 -O2 -o main.elf program2.c serial.c os.c 
	avr-objcopy -O ihex main.elf main.hex
	avr-size main.elf

#Flash the Arduino
#Be sure to change the device (the argument after -P) to match the device on your computer
#On Windows, change the argument after -P to appropriate COM port
program: main.hex
	avrdude -D -pm2560 -P /dev/tty.usbmodemFA1321 -c wiring -F -u -U flash:w:main.hex

#remove build files
clean:
	rm -fr *.elf *.hex *.o
