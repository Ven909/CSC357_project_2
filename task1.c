#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>

#define MAX_INODES 1024  // Maximum number of inodes
#define NAME_SIZE 32     // Maximum file/directory name length

// Structure representing an inode
typedef struct {
    uint32_t inode;       // Unique inode number
    uint32_t parentInode; // Parent inode number
    char type;            // 'd' for directory, 'f' for file
    char name[NAME_SIZE]; // Name of file or directory
} Inode;

Inode inodeList[MAX_INODES];  // Array of inodes (file system metadata)
size_t inodeCount = 0;        // Number of inodes currently in use
uint32_t currentInode = 0;    // Tracks the currently active directory

// Function prototypes
void loadInodeList(const char *path);
void saveInodeList(const char *path);
void changeDirectory(const char *name);
void listContents();
void createDirectory(const char *name);
void createFile(const char *name);
void trimNewline(char *str);

int main(int argc, char *argv[]) {
    if (argc != 2) { // Ensure correct number of arguments
        printf("Usage: %s <filesystem_directory>\n", argv[0]);
        return 1;
    }

    const char *fsPath = argv[1];
    chdir(fsPath);  // Change the working directory to the file system directory

    loadInodeList("inodes_list");  // Load existing inodes from file

    char command[64];
    while (1) {
        printf("> ");
        if (!fgets(command, sizeof(command), stdin)) break; // Read user input

        trimNewline(command); // Remove trailing newline character

        if (strcmp(command, "exit") == 0) { 
            saveInodeList("inodes_list"); // Save inode changes before exiting
            break;
        } else if (strncmp(command, "cd ", 3) == 0) { 
            changeDirectory(command + 3); // Change directory
        } else if (strcmp(command, "ls") == 0) { 
            listContents(); // List files and directories
        } else if (strncmp(command, "mkdir ", 6) == 0) { 
            createDirectory(command + 6); // Create a new directory
        } else if (strncmp(command, "touch ", 6) == 0) { 
            createFile(command + 6); // Create a new file
        } else {
            printf("Invalid command\n");
        }
    }
    return 0;
}

// Removes trailing newline from input strings
void trimNewline(char *str) {
    char *pos = strchr(str, '\n');
    if (pos) *pos = '\0'; // Replace newline with null terminator
}

// Loads inode metadata from the inodes_list file
void loadInodeList(const char *path) {
    FILE *file = fopen(path, "rb");  // Open file in binary mode
    if (!file) return;  // If file doesn't exist, assume empty inode list

    inodeCount = fread(inodeList, sizeof(Inode), MAX_INODES, file); // Read inodes
    fclose(file);
}

// Saves the inode list to the inodes_list file
void saveInodeList(const char *path) {
    FILE *file = fopen(path, "wb");  // Open file in binary mode for writing
    if (!file) return;

    fwrite(inodeList, sizeof(Inode), inodeCount, file); // Write all inodes
    fclose(file);
}

// Changes the current directory in the emulated file system
void changeDirectory(const char *name) {
    for (size_t i = 0; i < inodeCount; i++) { // Loop through all inodes
        if (inodeList[i].parentInode == currentInode && // Check if it's in the current directory
            inodeList[i].type == 'd' && // Ensure it's a directory
            strcmp(inodeList[i].name, name) == 0) { // Compare names
            currentInode = inodeList[i].inode; // Update current inode
            return;
        }
    }
    printf("Directory not found\n");
}

// Lists the contents (files and directories) of the current directory
void listContents() {
    for (size_t i = 0; i < inodeCount; i++) { // Iterate through inode list
        if (inodeList[i].parentInode == currentInode) { // If inode is in current directory
            printf("inode: %u, type: %c, name: %s\n",
                   inodeList[i].inode, inodeList[i].type, inodeList[i].name); // Print inode info
        }
    }
}

// Creates a new directory
void createDirectory(const char *name) {
    if (inodeCount >= MAX_INODES) { // Check if max inodes reached
        printf("Max inodes reached\n");
        return;
    }
    
    for (size_t i = 0; i < inodeCount; i++) { // Check if directory already exists
        if (inodeList[i].parentInode == currentInode && strcmp(inodeList[i].name, name) == 0) {
            printf("Directory already exists\n");
            return;
        }
    }

    // Create new directory inode
    Inode newDir = {inodeCount, currentInode, 'd', ""};
    strncpy(newDir.name, name, NAME_SIZE - 1); // Copy name to inode
    inodeList[inodeCount++] = newDir; // Add inode to inode list

    // Create an actual file for the directory
    char filename[16];
    snprintf(filename, sizeof(filename), "%u", newDir.inode); // Create filename
    FILE *file = fopen(filename, "w");
    if (file) {
        fprintf(file, "%u .\n%u ..\n", newDir.inode, currentInode); // Add "." and ".."
        fclose(file);
    }
}

// Creates a new regular file
void createFile(const char *name) {
    if (inodeCount >= MAX_INODES) { // Check if max inodes reached
        printf("Max inodes reached\n");
        return;
    }

    for (size_t i = 0; i < inodeCount; i++) { // Check if file already exists
        if (inodeList[i].parentInode == currentInode && strcmp(inodeList[i].name, name) == 0) {
            printf("File already exists\n");
            return; // Do nothing if file already exists
        }
    }

    // Create new file inode
    Inode newFile = {inodeCount, currentInode, 'f', ""}; // Create inode
    strncpy(newFile.name, name, NAME_SIZE - 1); // Copy name to inode
    inodeList[inodeCount++] = newFile; // Add inode to inode list

    // Create an actual file for the inode
    char filename[16];
    snprintf(filename, sizeof(filename), "%u", newFile.inode); // Create filename
    FILE *file = fopen(filename, "w");
    if (file) {
        fprintf(file, "%s\n", name); // Store the file name in the file
        fclose(file);
    }
}


