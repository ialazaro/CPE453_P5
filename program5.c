#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>

#include "ext2.h"
#include "SdReader.h"
#include "globals.h"
#include "os.h"
#include "synchro.h"
#include "program5.h"

super_block sb;

static song *songs;
static uint8_t curr_song = 0;
static song *curr;
static uint32_t song_time = 0;
static uint32_t song_start;

static int8_t *block;

int main(void) {
   uint8_t sd_card_status;
   inode data;
   dir_entry *entries;
   dir_entry *start;
   
   sd_card_status = sdInit(1);   //initialize the card with slow clock

   serial_init(); 
   clear_screen();   

   if (!sd_card_status) {
      set_cursor(1, 1);
      print_string("sd init failed...exiting");
      return 1;
   }

   
   block = calloc(sizeof(uint8_t), 1024);

   start_audio_pwm();
   os_init();

   /* read the super block */
   sdReadData(2, 0, (uint8_t *)&sb, sizeof(super_block));

   /* find the root inode */
   find_inode(&data, (uint32_t)2);

   sdReadData(data.i_block[0]*2, 0, (uint8_t *)block, (uint16_t)512);
   sdReadData(data.i_block[0]*2 + 1, 0, (uint8_t *)&block[512], (uint16_t)512);
   entries = (void *)block;
   start = entries;

   /* skip ., .., and lost+found */
   entries = (void *)entries + entries->rec_len;
   entries = (void *)entries + entries->rec_len;
   entries = (void *)entries + entries->rec_len;
   
   songs = calloc(sizeof(song), 1);
   curr = songs;

   /* read the songs into the array */
   while ((void *)entries != (void *)start + 1024) {      
      curr->name = calloc(sizeof(char), entries->name_len + 1);
      strncpy(curr->name, entries->name, entries->name_len);
      
      curr->inode = entries->inode;
      
      /* find the size of the file */
      find_inode(&data, curr->inode);

      curr->size = data.i_size/20480;

      entries = (void *)entries + entries->rec_len;
      if ((void *)entries != (void *)start + 1024) {
         curr->next = calloc(sizeof(song), 1);
         curr->next->prev = curr;
         curr = curr->next;
      }
   }
   curr->next = songs;
   songs->prev = curr;
   
   curr = songs;

   /* initialize mutex */

   /* create_threads */
   create_thread("playback", (uint16_t)playback, NULL, 64);
   create_thread("read",     (uint16_t)read,     NULL, 64);
   create_thread("display",  (uint16_t)display,  NULL, 64);
   create_thread("idle",     (uint16_t)idle,     NULL, 16);

   os_start();

   return 0;
}

/*
 * play the audio from the buffer
 * thread 0
 */
void playback(void) {
   
   
   while (1) {
      
   }
}

/*
 * read the SD card and fill the buffer
 * thread 1
 */
void read(void) {
   
   
   while (1) {
      
   }
}

/*
 * display the information to the console
 * thread 2
 */
void display(void) {
   while (1) {
      
      handle_keys();
      
      set_color(YELLOW);
      set_cursor(1, 1);
      print_string("Program 5");
      
      /* print song info */
      set_cursor(2, 1);
      /* clear the line */
      print_string("                                                                            ");
      set_cursor(2, 1);
      print_string(curr->name);

      set_cursor(3, 1);
      print_string("song length: ");
      print_int32(curr->size);

      set_cursor(4, 1);
      print_string("current time: ");
      print_int32(get_time() - song_start);
      

      set_cursor(5, 1);
      set_color(32);
      print_string("Time: ");
      print_int32(get_time());
      
      set_cursor(6, 1);
      set_color(36);
      print_string("Number of threads: ");
      print_int(get_num_threads());
      
      set_cursor(9, 1);
      set_color(37);
      print_thread_info();
   }
}

/*
 * take up extra time in cycle
 * thread 3
 */
void idle(void) {
   while (1);
}

/*
 * handle the key presses to change songs
 */

void handle_keys(void) {
   uint8_t input = read_byte();

   if (input != 255) {
      if (input == 'n') {
         /* go to next song */
         curr = curr->next;
         
         /* reset the song timer */
         song_start = get_time();
      }
      else if (input == 'p') {
         /* go to prev song */
         curr = curr->prev;
         
         song_start = get_time();
      }


   }
}
