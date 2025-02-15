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

Inode inodeList[MAX_INODES];  
size_t inodeCount = 0;  
uint32_t currentInode = 0;  

void loadInodeList(const char *path);
void saveInodeList(const char *path);
void changeDirectory(const char *name);
void listContents();
void createDirectory(const char *name);
void createFile(const char *name);
void trimNewline(char *str);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <filesystem_directory>\n", argv[0]);
        return 1;
    }

    const char *fsPath = argv[1];
    chdir(fsPath);

    loadInodeList("inodes_list");

    char command[64];
    while (1) {
        printf("> ");
        if (!fgets(command, sizeof(command), stdin)) break;

        trimNewline(command);

        if (strcmp(command, "exit") == 0) {
            saveInodeList("inodes_list");
            break;
        } else if (strncmp(command, "cd ", 3) == 0) {
            changeDirectory(command + 3);
        } else if (strcmp(command, "ls") == 0) {
            listContents();
        } else if (strncmp(command, "mkdir ", 6) == 0) {
            createDirectory(command + 6);
        } else if (strncmp(command, "touch ", 6) == 0) {
            createFile(command + 6);
        } else {
            printf("Invalid command\n");
        }
    }
    return 0;
}

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
