#Change this variable to point to your Arduino device
#Mac - it may be different
DEVICE = /dev/cu.usbmodem1451

#Linux (/dev/ttyACM0 or possibly /dev/ttyUSB0)
#DEVICE = /dev/ttyACM0 

#Windows
#DEVICE = COM3 

#Compile the code
main: program5.c serial.c os.c synchro.c os_util.c fs_util.c SdReader.c
	avr-gcc -mmcu=atmega2560 -DF_CPU=16000000 -O2 -o main.elf program5.c serial.c os.c synchro.c os_util.c fs_util.c SdReader.c
	avr-objcopy -O ihex main.elf main.hex
	avr-size main.elf

program_5: program5.c serial.c os.c synchro.c os_util.c fs_util.c SdReader.c
	avr-gcc -mmcu=atmega328p -DF_CPU=16000000 -O2 -o main.elf program5.c serial.c os.c synchro.c os_util.c fs_util.c SdReader.c
	avr-objcopy -O ihex main.elf main.hex
	avr-size main.elf

#Flash the Arduino
#Be sure to change the device (the argument after -P) to match the device on your computer
#On Windows, change the argument after -P to appropriate COM port
program: main.hex
	avrdude -D -pm2560 -P $(DEVICE) -c wiring -F -u -U flash:w:main.hex

screen:
	screen $(DEVICE) 115200

#remove build files
clean:
	rm -fr *.elf *.hex *.o
