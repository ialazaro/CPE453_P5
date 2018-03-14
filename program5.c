#include <avr/io.h>
#include <avr/interrupt.h>
#include "globals.h"
#include "os.h"
#include <util/delay.h>
#include <stdlib.h>
#include "ext2.h"
#include "SdReader.h"

int main(void) {
   uint8_t sd_card_status;

   sd_card_status = sdInit(1);   //initialize the card with slow clock


   //serial_init(); 

   //start_audio_pwm();
   //os_init();
   
   return 0;
}
