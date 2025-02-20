#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "task1.h"

Inode inodeList[MAX_INODES];
size_t inodeCount = 0;
uint32_t currentInode = 0;

void trimNewline(char *str) {
    char *pos = strchr(str, '\n');
    if (pos) *pos = '\0';
}

void loadInodeList(const char *path) {
    FILE *file = fopen(path, "rb");
    if (!file) return;

    inodeCount = fread(inodeList, sizeof(Inode), MAX_INODES, file);
    fclose(file);
}

void saveInodeList(const char *path) {
    FILE *file = fopen(path, "wb");
    if (!file) return;

    fwrite(inodeList, sizeof(Inode), inodeCount, file);
    fclose(file);
}

void changeDirectory(const char *name) {
    for (size_t i = 0; i < inodeCount; i++) {
        if (inodeList[i].parentInode == currentInode &&
            inodeList[i].type == 'd' && strcmp(inodeList[i].name, name) == 0) {
            currentInode = inodeList[i].inode;
            return;
        }
    }
    printf("Directory not found\n");
}

void listContents() {
    for (size_t i = 0; i < inodeCount; i++) {
        if (inodeList[i].parentInode == currentInode) {
            printf("inode: %u, type: %c, name: %s\n",
                   inodeList[i].inode, inodeList[i].type, inodeList[i].name);
        }
    }
}

void createDirectory(const char *name) {
    if (inodeCount >= MAX_INODES) {
        printf("Max inodes reached\n");
        return;
    }
    for (size_t i = 0; i < inodeCount; i++) {
        if (inodeList[i].parentInode == currentInode && strcmp(inodeList[i].name, name) == 0) {
            printf("Directory already exists\n");
            return;
        }
    }

    Inode newDir = {inodeCount, currentInode, 'd', ""};
    strncpy(newDir.name, name, NAME_SIZE - 1);
    inodeList[inodeCount++] = newDir;

    char filename[16];
    snprintf(filename, sizeof(filename), "%u", newDir.inode);
    FILE *file = fopen(filename, "w");
    if (file) {
        fprintf(file, "%u .\n%u ..\n", newDir.inode, currentInode);
        fclose(file);
    }
}

void createFile(const char *name) {
    if (inodeCount >= MAX_INODES) {
        printf("Max inodes reached\n");
        return;
    }
    for (size_t i = 0; i < inodeCount; i++) {
        if (inodeList[i].parentInode == currentInode && strcmp(inodeList[i].name, name) == 0) {
            printf("File already exists\n");
            return;
        }
    }

    Inode newFile = {inodeCount, currentInode, 'f', ""};
    strncpy(newFile.name, name, NAME_SIZE - 1);
    inodeList[inodeCount++] = newFile;

    char filename[16];
    snprintf(filename, sizeof(filename), "%u", newFile.inode);
    FILE *file = fopen(filename, "w");
    if (file) {
        fprintf(file, "%s\n", name);
        fclose(file);
    }
}
