//Global defines
#ifndef GLOBALS_H
#define GLOBALS_H

#define RED 31
#define GREEN 32
#define YELLOW 33
#define BLUE 34
#define MAGENTA 35
#define CYAN 36
#define WHITE 37

void serial_init();
uint8_t byte_available();
uint8_t read_byte();
uint8_t write_byte(uint8_t b);
void print_string(uint8_t* s);
void print_int(uint16_t i);
void print_int32(uint32_t i);
void print_hex(uint16_t i);
void print_hex32(uint32_t i);
void set_cursor(uint8_t row, uint8_t col);
void set_color(uint8_t color);
void clear_screen(void);

void print_thread_info(void);
//place defines and prototypes here

#endif
