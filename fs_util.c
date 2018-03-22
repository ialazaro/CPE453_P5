/*
 * Nicholas Berriochoa
 * 9 March 2018
 * CSC 453: Assignment 4
 * Description: Read a linux ext2 file system
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* strcmp */

#include "program4.h"
#include "ext2.h"
#include "globals.h"

#define MAX_OFFSET 512

FILE *fp;
extern super_block sb;

int other_main(int argc, char **argv) {
   int opt, cat = 0;
   char *image = NULL, *path = NULL;
   
   inode data;

   /* argument checker */
   if (argc < MIN_ARGS || argc > MAX_ARGS) {
      print_usage();
      exit(EXIT_FAILURE);
   }

   /* argument parser */
   /*while ((opt = getopt(argc, argv, "l")) != -1) {
      switch (opt) {
         case 'l': 
            cat = 1;
            if (argc != MAX_ARGS) {
               print_usage();
               exit(EXIT_FAILURE);
            }

            break;
         default:
            print_usage();
            exit(EXIT_FAILURE);
      }
   }*/
   
   /* set the image and path assuming proper order */
   image = argv[1 + cat];
   path = (argc > MIN_ARGS)? argv[2 + cat] : ROOT_PATH;

   /*if (!(fp = fopen(image, PERMS))) {
      fprintf(stderr, "file %s not found\n", image);
      exit(EXIT_FAILURE);
   }*/
      
   /* read the super block */
   sdReadData(2, 0, (uint8_t *)&sb, sizeof(super_block));

   /* get the root inode */
   find_inode(&data, EXT2_ROOT_INO);   

   if (strcmp(path, ROOT_PATH)) {
      /* 
       * if the path is not at the root, do some traversing to 
       * find the correct inode
       * exit if the file/path cannot be found
       */
      traverse(&data, path);      
   }
 
   /* all is well so print the data */
   /* check the file type */
   if ((data.i_mode & MODE_MASK) == EXT2_S_IFDIR) {
      printf(FORMAT_HEADER);
      print_dir(data);
   }
   else if ((data.i_mode & MODE_MASK) == EXT2_S_IFREG) {
      print_reg(data);
   }
   else {
      fprintf(stderr, "Inode type 0x%04X not supported\n", 
               data.i_mode & MODE_MASK);
      fclose(fp);
      exit(EXIT_FAILURE);
   }

   fclose(fp);
   
   return EXIT_SUCCESS;
}

void print_usage(void) {
   fprintf(stderr, "usage: ext2reader image [[-l] path]\n");
   fprintf(stderr, "\tl: print to the screen the contents of path\n");
}

/*
 * find the inode based on the inode number
 */
void find_inode(inode *data, uint32_t inode_number) {
   /* https://wiki.osdev.org/Ext2 */
   
   uint32_t block_group      = (inode_number - 1) / sb.s_inodes_per_group;
   uint32_t index            = (inode_number - 1) % sb.s_inodes_per_group;
   uint32_t containing_block = (index * sizeof(inode)) / MAX_OFFSET;
   uint32_t offset           = index * sizeof(inode) % MAX_OFFSET;
   uint32_t block            = BASE_BLOCKS +
                               (2*sb.s_blocks_per_group * block_group) +
                               containing_block;
   uint8_t ret;
   
   
   set_cursor(1, 1);
   print_string("block group: ");
   print_int32(block_group);
   
   set_cursor(2, 1);
   print_string("index: ");
   print_int32(index);
   
   set_cursor(3, 1);
   print_string("containing: ");
   print_int32(containing_block);
   
   set_cursor(4, 1);
   print_string("offset: ");
   print_int32(offset);
   
   set_cursor(5, 1);
   print_string("block: ");
   print_int32(block);
   
   set_cursor(6, 1);
   print_int32(inode_number);

   ret = sdReadData(block, offset, (uint8_t *)data, sizeof(inode));
   
   set_cursor(7, 1);
   print_int(sizeof(inode) + offset);
}

void traverse(inode *data, char *path) {
   
   dir_entry *entry;
   uint8_t block[BLOCK_SIZE];
   uint8_t curr_block = 0;
   uint8_t found = 0;
   
   char *str = calloc(sizeof(char), strlen(path));
   strncpy(str, path, strlen(path));

   const char delim[2] = ROOT_PATH;
   char *token;
   char *name;

   token = strtok(str, delim);

   while (token != NULL) {
      
      sdReadData(data->i_block[curr_block]*2, 0, (uint8_t *)block, BLOCK_SIZE);
      entry = (dir_entry *)block;

      while ((void *)entry != (void *)&block[BLOCK_SIZE]) {
         name = calloc(sizeof(char), entry->name_len);
         strncpy(name, entry->name, entry->name_len);

         if (!(strcmp(token, name))) {
            find_inode(data, entry->inode);
            found = 1;
            
            break;
         }

         free(name);
         
         entry = (void *)entry + entry->rec_len;
      }

      if (!found) {
         fprintf(stderr, "%s not found\n", path);
         exit(EXIT_FAILURE);
      }

      token = strtok(NULL, delim);
   }
   free(str);
}

void print_dir(inode data) {
   uint8_t block[BLOCK_SIZE];
   uint8_t curr_block = 0;
   uint8_t s_indirect[BLOCK_SIZE];
   uint8_t d_indirect[BLOCK_SIZE];
   uint16_t curr_addr;
   uint16_t curr_i_addr;
   uint32_t *address;
   uint32_t *i_address;
   inode meta;
   dir_entry *entry;
   
   /* check the direct blocks */
   for (curr_block = 0; curr_block < EXT2_NDIR_BLOCKS; curr_block++) {
      sdReadData(data.i_block[curr_block]*2, 0, (uint8_t *)block, BLOCK_SIZE);
      entry = (void *)block;
      
      while (entry->inode) {
         print_name(entry->name, entry->name_len);
         find_inode(&meta, entry->inode);
         print_meta_data(meta);
         
         entry = (void *)entry + entry->rec_len;
         
         if ((void *)entry == (void *)&block[1024]) {
            
         }
      }
   }
   
   if (data.i_size < (BLOCK_SIZE*EXT2_NDIR_BLOCKS)) {
    
      return;
   }
   
   /* check the singly indirect blocks */
   sdReadData(data.i_block[EXT2_NDIR_BLOCKS]*2, 0, (uint8_t *)s_indirect, BLOCK_SIZE);
   address = (void *)s_indirect;
   
   for (curr_addr = 0; curr_addr < ADDR_PER_BLOCK; curr_addr++) {
      sdReadData(address[curr_addr]*2, 0, (uint8_t *)block, BLOCK_SIZE);
      
      entry = (void *)block;
      while ((void *)entry != (void *)&block[BLOCK_SIZE]) {
         print_name(entry->name, entry->name_len);
         find_inode(&meta, entry->inode);
         print_meta_data(meta);
         
         entry = (void *)entry + entry->rec_len;
      }
   }
   
   /* file was contained within the 12 direct blocks and SIB */
   /*if (data.i_size < ((BLOCK_SIZE*ADDR_PER_BLOCK) + (BLOCK_SIZE*EXT2_NDIR_BLOCKS))) {
     
      return;
   }*/
   
   /* check the doubly indirect blocks */
   sdReadData(data.i_block[EXT2_DIND_BLOCK]*2, 0, (uint8_t *)d_indirect, BLOCK_SIZE);
   i_address = (void *)d_indirect;
   
   for (curr_i_addr = 0; curr_i_addr < ADDR_PER_BLOCK; curr_i_addr++) {
      sdReadData(i_address[curr_i_addr]*2, 0, (uint8_t *)s_indirect, BLOCK_SIZE);
      address = (void *)s_indirect;
      
      for (curr_addr = 0; curr_addr < ADDR_PER_BLOCK; curr_addr++) {
         sdReadData(address[curr_addr]*2, 0, (uint8_t *)block, BLOCK_SIZE);
         
         entry = (void *)block;
         while ((void *)entry != (void *)&block[BLOCK_SIZE]) {
            print_name(entry->name, entry->name_len);
            find_inode(&meta, entry->inode);
            print_meta_data(meta);
            
            entry = (void *)entry + entry->rec_len;
         }
      }
   }
   
}

/*
 * prints the name without having to null terminate the string
 * NOTE: formatting will break if the name is longer than 20 characters
 */
void print_name(char *name, uint16_t name_len) {
   uint8_t spaces = NAME_SEP;
   uint8_t i;

   for (i = 0; i < name_len; i++, spaces--) {
      printf("%c", name[i]);
   }

   while (spaces--) {
      printf(" ");
   }
}

void print_reg(inode data) {
   uint8_t block[BLOCK_SIZE];
   uint8_t curr_block = 0;
   uint8_t s_indirect[BLOCK_SIZE];
   uint8_t d_indirect[BLOCK_SIZE];
   uint16_t curr_addr;
   uint16_t curr_i_addr;
   uint32_t *address;
   uint32_t *i_address;

   /* check the direct blocks */
   uint16_t i;
   for (curr_block = 0; curr_block < EXT2_NDIR_BLOCKS; curr_block++) {
      sdReadData(data.i_block[curr_block]*2, 0, (uint8_t *)block, BLOCK_SIZE);

      for (i = 0; i < BLOCK_SIZE; i++) {
         printf("%c", block[i]);
      }
   }
   
   /* file was contained within the 12 direct blocks */
   if (data.i_size < (BLOCK_SIZE*EXT2_NDIR_BLOCKS)) {
      return;
   }

   /* check the singly indirect blocks */
   sdReadData(data.i_block[EXT2_DIND_BLOCK]*2, 0, (uint8_t *)s_indirect, BLOCK_SIZE);
   address = (void *)s_indirect;

   for (curr_addr = 0; curr_addr < ADDR_PER_BLOCK; curr_addr++) {
      sdReadData(address[curr_addr]*2, 0, (uint8_t *)block, BLOCK_SIZE);

      for(i = 0; i < BLOCK_SIZE; i++) {
         printf("%c", block[i]);
      }
   }

   /* file was contained within the 12 direct blocks and SIB */
   /*if (data.i_size < ((BLOCK_SIZE*ADDR_PER_BLOCK) + (BLOCK_SIZE*EXT2_NDIR_BLOCKS))) {
      return;
   }*/
   
   /* check the doubly indirect blocks */
   sdReadData(data.i_block[EXT2_DIND_BLOCK]*2, 0, (uint8_t *)d_indirect, BLOCK_SIZE);
   i_address = (void *)d_indirect;

   for (curr_i_addr = 0; curr_i_addr < ADDR_PER_BLOCK; curr_i_addr++) {
      sdReadData(i_address[curr_i_addr]*2, 0, (uint8_t *)s_indirect, BLOCK_SIZE);
      address = (void *)s_indirect;

      for (curr_addr = 0; curr_addr < ADDR_PER_BLOCK; curr_addr++) {
         sdReadData(address[curr_addr]*2, 0, (uint8_t *)block, BLOCK_SIZE);

         for (i = 0; i < BLOCK_SIZE; i++) {
            printf("%c", block[i]);
         }
      }
   }

   /*no support for TIB */
}



/*
 * Prints the meta data for the inode (size and type)
 */
void print_meta_data(inode meta) {
   if ((meta.i_mode & MODE_MASK) == EXT2_S_IFDIR) {
      
      /* no size for directories */
      printf("       0");
      
      /* print type */
      printf("        D\n");
   }
   else if ((meta.i_mode & MODE_MASK) == EXT2_S_IFREG) {
      
      /* print size */
      printf("%8d", meta.i_size);
      
      /* print type */
      printf("        F\n");
   }
   else {
      printf("%d", meta.i_size);
      printf("invalid type 0x%04X\n", meta.i_mode & MODE_MASK);
   }
}

