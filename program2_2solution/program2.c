#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "globals.h"
#include "os.h"

uint8_t send_int(uint16_t x);

void led_on() {
   asm volatile ("LDI R31, 0x00");
   asm volatile ("LDI R30, 0x24");
   asm volatile ("LD R25, Z"); // Load (Z) into R25.
   asm volatile ("SBR R25, 0x80"); // Set bit 5.
   asm volatile ("ST Z, R25"); // Store into Z location whatever is in R25.

   asm volatile ("LDI R31, 0x00");
   asm volatile ("LDI R30, 0x25");
   asm volatile ("LD R25, Z"); // Load (Z) into R25.
   asm volatile ("SBR R25, 0x80"); // Set bit 5.
   asm volatile ("ST Z, R25"); // Store into Z location whatever is in R25.
}

void led_off() {
   asm volatile ("LDI R31, 0x00");
   asm volatile ("LDI R30, 0x24");
   asm volatile ("LD R25, Z"); // Load (Z) into R25.
   asm volatile ("SBR R25, 0x80"); // Set bit 5.
   asm volatile ("ST Z, R25"); // Store into Z location whatever is in R25.

   asm volatile ("LDI R31, 0x00");
   asm volatile ("LDI R30, 0x25");
   asm volatile ("LD R25, Z"); // Load (Z) into R25.
   asm volatile ("CBR R25, 0x80"); // Cclear bit 5.
   asm volatile ("ST Z, R25"); // Store into Z location whatever is in R25.
}

void blink(uint16_t *delay) {
   uint16_t i=*delay;
   uint16_t j=i;

   while(1) {
      i=j;
      led_off();
      while(i--)
         _delay_ms(1);

      i=j;
      led_on();
      while(i--)
         _delay_ms(1);
   }
}

void stats(uint8_t *str) {
   while(1) {
      set_color(YELLOW);
      set_cursor(1, 1);
      print_string(str);

      set_cursor(2, 1);
      set_color(32);
      print_string("Time: ");
      send_int(get_time());

      set_cursor(5, 1);
      set_color(36);
      print_string("Number of threads: ");
      print_int(get_num_threads());

      set_cursor(6, 1);
      set_color(37);
      print_thread_info();
   }
}

void main(void) {
   serial_init();

   uint16_t delay = 50;
   uint8_t string[15] = "Program 2";

   os_init();

   create_thread("stats", (uint16_t)stats, (void*)string, 200);
   create_thread("blink", (uint16_t)blink, (void*)&delay, 200);

   _delay_ms(100);
   clear_screen();

   os_start();
}
