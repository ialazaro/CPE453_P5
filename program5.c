#include <avr/io.h>
#include <avr/interrupt.h>
#include "globals.h"
#include "os.h"
#include <util/delay.h>
#include <stdlib.h>
#include "ext2.h"
#include "SdReader.h"

extern volatile struct system_t *system;

/* buffers and index */
static uint8_t buffer1[BUFFER_SIZE] = {0};
static uint8_t buffer2[BUFFER_SIZE] = {0};
static uint8_t index1 = 0;
static uint8_t index2 = 0;
static uint8_t playing = 0;

static struct mutex_t b1;
static struct mutex_t b2;

static struct ext2_dir_entry *entries[8];
static uint8_t dir_index = 0;

int main(void) {
   uint8_t sd_card_status;

   sd_card_status = sdInit(1);   //initialize the card with slow clock

   serial_init(); 

   start_audio_pwm();
   os_init();
   
   return 0;
}



void idle(void) {
   while (1);
}

void display(void) {
   while (1) {

   }
}

void player(void) {
   struct mutex_t *play;
   
   while (1) {
      
      if (playing == 0) {
         playing = b1;
      }
      else if (playing == 1) {
         playing = b2;
      }

      mutex_lock(play);

      OCR2B = (!playing) ? buffer1[index1] : buffer2[index2];

      mutex_unlock(play);

      yield();
   }
}

void reader(void) {
   struct ext2_inode song;
   struct mutex_t *read;

   find_inode(&song, entries[dir_index]->inode);
   
   
   while (1) {

   }
}
