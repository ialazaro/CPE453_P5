#ifndef PROGRAM5_H
#define PROGRAM5_H

typedef struct song {
   uint32_t inode;
   char *name;
   struct song *next;
   struct song *prev;
} song;

/* main threads */
void display(void);
void playback(void);
void read(void);
void idle(void);

/* helper functions */
void handle_keys(void);

#endif

