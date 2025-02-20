#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define MAX_INODES 1024
#define NAME_SIZE 32

typedef struct {
    uint32_t inode;
    uint32_t parentInode;
    char type;
    char name[NAME_SIZE];
} Inode;

// Function prototypes
void loadInodeList(const char *path);
void saveInodeList(const char *path);
void changeDirectory(const char *name);
void listContents();
void createDirectory(const char *name);
void createFile(const char *name);
void trimNewline(char *str);

#endif
