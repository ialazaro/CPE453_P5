DEVICE = /dev/tty.usbmodemFA131

#Compile the code
main: program5.c
	avr-gcc -mmcu=atmega2560 -DF_CPU=16000000 -O2 -o main.elf program3.c serial.c os.c synchro.c
	avr-objcopy -O ihex main.elf main.hex
	avr-size main.elf
	#avr-objdump -d main.elf | less

#program_3: program3.c serial.c os.c synchro.c
#	avr-gcc -mmcu=atmega2560 -DF_CPU=16000000 -O2 -o main.elf program3.c serial.c os.c synchro.c
#	avr-objcopy -O ihex main.elf main.hex
#	avr-size main.elf

#Flash the Arduino
#Be sure to change the device (the argument after -P) to match the device on your computer
#On Windows, change the argument after -P to appropriate COM port
program: main.hex
	avrdude -D -pm2560 -P /dev/tty.usbmodemFA131 -c wiring -F -u -U flash:w:main.hex
#remove build files

#handin: README
#	handin jseng 453_program3 Makefile synchro.c synchro.h program3.c os.c os.h globals.h serial.c README

clean:
	rm -fr *.elf *.hex *.o
