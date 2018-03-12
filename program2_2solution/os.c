#include <avr/io.h>
#include <avr/interrupt.h>
#include "globals.h"
#include "os.h"
#include <stdlib.h>
#include <string.h>

typedef struct thread_t {
   uint8_t* stackPtr;
   uint8_t thread_id;
   char name[10];
   uint16_t func;
   uint8_t *stack;
   uint8_t *stackBase;
   uint16_t size;
} thread_t;

typedef struct system_t {
   thread_t array[8];
   uint8_t threadsUsed;
   uint8_t currThread;
   uint16_t num_interrupts;
} system_t;

volatile static system_t sysArray;

uint16_t get_time() {
    return sysArray.num_interrupts / 100;
}

uint8_t get_next_thread() {
   return (sysArray.currThread+1) % sysArray.threadsUsed;
}

uint8_t get_num_threads() {
    return sysArray.threadsUsed;
}

void print_thread_info() {
   uint8_t i, cursor_position = 6, stack_usage;

   for(i = 0; i < sysArray.threadsUsed; i++) {
      set_cursor(cursor_position++, 1);
      print_string("Thread ID: ");
      print_int(i);

      set_cursor(cursor_position++, 1);
      print_string("Thread name: ");
      print_string((char*)sysArray.array[i].name);
      
      set_cursor(cursor_position++, 1);
      print_string("Thread PC: 0x");
      print_hex(sysArray.array[i].func);

      stack_usage = sysArray.array[i].stackBase - 
      sysArray.array[i].stackPtr - 1;

      set_cursor(cursor_position++, 1);
      print_string("Stack Usage: ");
      print_int(stack_usage);
      print_string(" bytes");

      set_cursor(cursor_position++, 1);
      print_string("Stack Size: ");
      print_int(sysArray.array[i].size);
      print_string(" bytes");

      set_cursor(cursor_position++, 1);
      print_string("Current top of stack: 0x");
      print_hex((uint16_t)sysArray.array[i].stackPtr);

      set_cursor(cursor_position++, 1);
      print_string("Stack base: 0x");
      print_hex((uint16_t)sysArray.array[i].stack);

      set_cursor(cursor_position++, 1);
      print_string("Stack end: 0x");
      print_hex((uint16_t)sysArray.array[i].stackBase);

      set_cursor(cursor_position++, 1);
   }
}

__attribute__((naked)) void context_switch(uint16_t* new_tp, uint16_t* old_tp) {
   asm volatile("push R2");
   asm volatile("push R3");
   asm volatile("push R4");
   asm volatile("push R5");
   asm volatile("push R6");
   asm volatile("push R7");
   asm volatile("push R8");
   asm volatile("push R9");
   asm volatile("push R10");
   asm volatile("push R11");
   asm volatile("push R12");
   asm volatile("push R13");
   asm volatile("push R14");
   asm volatile("push R15");
   asm volatile("push R16");
   asm volatile("push R17");
   asm volatile("push R28");
   asm volatile("push R29");

   //--------
   asm volatile("ldi r31,0");
   asm volatile("ldi r30,0x5d");
   asm volatile("ld r17,Z+");
   asm volatile("ld r18,Z");

   asm volatile("mov r30,r22");
   asm volatile("mov r31,r23");
   asm volatile("st Z+,r17");
   asm volatile("st Z,r18");
   //--------

   asm volatile("mov r30,r24");
   asm volatile("mov r31,r25");
   asm volatile("ld r17,Z+");
   asm volatile("ld r18,Z");

   asm volatile("ldi r31,0");
   asm volatile("ldi r30,0x5d");
   asm volatile("st Z+,r17");
   asm volatile("st Z,r18");
   //--------

   asm volatile("pop R29");
   asm volatile("pop R28");
   asm volatile("pop R17");
   asm volatile("pop R16");
   asm volatile("pop R15");
   asm volatile("pop R14");
   asm volatile("pop R13");
   asm volatile("pop R12");
   asm volatile("pop R11");
   asm volatile("pop R10");
   asm volatile("pop R9");
   asm volatile("pop R8");
   asm volatile("pop R7");
   asm volatile("pop R6");
   asm volatile("pop R5");
   asm volatile("pop R4");
   asm volatile("pop R3");
   asm volatile("pop R2");
   asm volatile("ret"); //return to ISR
}

//This interrupt routine is automatically run every 10 milliseconds
ISR(TIMER0_COMPA_vect) {
   //The following statement tells GCC that it can use registers r18-r31,
   //for this interrupt routine.  These registers (along with r0 and r1) 
   //will automatically be pushed and popped by this interrupt routine.
   asm volatile ("" : : : "r18", "r19", "r20", "r21", "r22", "r23", "r24", \
                 "r25", "r26", "r27", "r30", "r31");

   uint8_t curr = sysArray.currThread;
   uint8_t nt = get_next_thread();

   sysArray.num_interrupts++;
   sysArray.currThread = nt;

   context_switch((uint16_t*)&sysArray.array[nt].stackPtr,
    (uint16_t*)&sysArray.array[curr].stackPtr);
}

//Call this to start the system timer interrupt
void start_system_timer() {
   TIMSK0 |= _BV(OCIE0A);  //interrupt on compare match
   TCCR0A |= _BV(WGM01);   //clear timer on compare match

   //Generate timer interrupt every ~10 milliseconds
   TCCR0B |= _BV(CS02) | _BV(CS00);    //prescalar /1024
   OCR0A = 156;             //generate interrupt every 9.98 milliseconds
}

void os_init() {
   int i;
   sysArray.threadsUsed = 0;
   sysArray.currThread = -1;
   sysArray.num_interrupts = 0;

   for(i=0;i<8;i++) {
      sysArray.array[i].func = 0;
      sysArray.array[i].stack = NULL;
      sysArray.array[i].stackPtr = NULL;
   }
}

void os_start() {
   sysArray.currThread = 0;

   // start system timer
   start_system_timer();
   sei();

   //context switch
   uint16_t temp;
   context_switch((uint16_t*)&sysArray.array[0].stackPtr,&temp);
}

__attribute__((naked)) void thread_start(void) {
   sei(); //enable interrupts - leave this as the first statement in thread_start()
   // Load in arguments from R29:R28 to R25:R24.
   // That's where the argument pointer goes.
   asm volatile("MOVW R24,R14");
   asm volatile("MOVW R30,R16"); // Put function pointer in Z.
   asm volatile("IJMP");
}

void create_thread(char* name, uint16_t address, void* args, uint16_t stack_size) {
   sysArray.array[sysArray.threadsUsed].thread_id = sysArray.threadsUsed;
   strcpy((char*)sysArray.array[sysArray.threadsUsed].name,name);

   sysArray.array[sysArray.threadsUsed].func = address;

   // Malloc space for the stack.
   sysArray.array[sysArray.threadsUsed].size = sizeof(regs_context_switch) +
    sizeof(regs_interrupt) + 32 + stack_size;
   sysArray.array[sysArray.threadsUsed].stack = 
    malloc(sysArray.array[sysArray.threadsUsed].size);

   // Move stack pointer to the top.
   sysArray.array[sysArray.threadsUsed].stackPtr = 
    sysArray.array[sysArray.threadsUsed].stack +
    sizeof(regs_context_switch) + sizeof(regs_interrupt) + 32 + stack_size;

   // Record high address of stack.
   sysArray.array[sysArray.threadsUsed].stackBase =
    sysArray.array[sysArray.threadsUsed].stackPtr;

   // Move stack pointer to where it needs to be to pop the registers.
   sysArray.array[sysArray.threadsUsed].stackPtr -= sizeof(regs_context_switch);
   sysArray.array[sysArray.threadsUsed].stackPtr -= 2;

   // Prepare the stack for context_switch.
   regs_context_switch *ptr = 
    (void*)sysArray.array[sysArray.threadsUsed].stackPtr;
   ptr->pcl = (uint16_t)thread_start&0xFF;
   ptr->pch = (uint16_t)thread_start>>8;
   ptr->eind= (uint16_t)0;

   ptr->r16 = address&0xFF;
   ptr->r17 = address>>8;
   ptr->r14 = (uint16_t)args&0xFF;
   ptr->r15 = (uint16_t)args>>8;

   // Count new thread in threadsUsed count.
   sysArray.threadsUsed++;
}
