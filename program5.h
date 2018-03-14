#ifndef PROGRAM5_H
#define PROGRAM5_H

#define BUFFER_SIZE 255

typedef struct fileInfo {
   char *name;
   char *path;
   uint32_t iNode;
   uint16_t len; //may not need
   struct fileInfo *next;
} fileInfo;

void idle(void);
void display(void);
void reader(void);
void player(void);

#endif

