#include <avr/io.h>
#include <avr/interrupt.h>
#include "globals.h"
#include "os.h"
#include <util/delay.h>
#include <stdlib.h>
#include "ext2.h"
#include "SdReader.h"

extern volatile struct system_t *system;

static fileInfo *songs;

/* buffers and index */
static uint8_t buffer1[BUFFER_SIZE] = {0};
static uint8_t buffer2[BUFFER_SIZE] = {0};
static uint8_t index1 = 0;
static uint8_t index2 = 0;

static uint8_t buff_num = 1;

static semaphore empty; //init to 256
static semaphore full; //init to 0
static mutex access;

//static uint8_t playing = 0;

static struct ext2_dir_entry *entries[13];
static uint8_t dir_index = 0;

int main(void) {
   uint8_t sd_card_status;
   inode data;

   sd_card_status = sdInit(1);   //initialize the card with slow clock

   serial_init(); 

   start_audio_pwm();
   os_init();
   
   // find the root
   find_inode(&data, 2);
   read_entries(data);

   os_start();

   return 0;
}

void read_entries(inode data) {
   uint8_t block[1024];
   dir_entry *entry;
   fileInfo *curr;

   read_data(data.i_block[0]*2, 0, (uint8_t *)block, 1024);   
   entry = (void *)block; 
   
   songs = malloc(sizeof(fileInfo));
   songs->inode = entry->inode;
   songs->name = entry->name;
   songs->len = entry->name_len;
   songs->next = NULL;

   curr = songs;

   while (entry->inode) {
      entry = (void *)entry + entry->rec_len;

      if ((void *)entry == (void *)&block[1024]) {
         break;
      }
      curr->next = malloc(sizeof(fileInfo));
      curr = curr->next;
      curr->inode = entry->inode;
      curr->name = entry->name;
      curr->len = entry->name_len;
      curr->next = NULL;
   } 

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
   uint8_t block[1024];
   uint16_t b_index = 0;

   find_inode(&song, songs->inode);
   read_data(data.i_block[0]*2, 0, (uint8_t *)block, 1024);

   while (1) {
      if (buff_num == 1) {
         // fill in the first buffer
         
         sem_wait(&empty);
         mutex_lock(&access);

         //read in the data
         buffer1[index1++] = block[b_index++];

         if (index1 >= 256) {
            //switch buffers
            buff_num = 2;
            index1 = 0;
         }

         mutex_unlock(&access);
         sem_signal(&full);

      }
      else if (buff_num == 2) {
         // fill in the second buffer
      }
      else {
         //bad stuff
      }
   }
}

