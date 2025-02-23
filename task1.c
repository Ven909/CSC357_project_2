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
    FILE *file = fopen(path, "rb"); // Open file in binary mode
    if (!file) {                     // If file doesn't exist, assume empty inode list
        printf("Error: inodes_list file not found. Initializing empty file system.\n");
        return;
    }

    // Read the entire inode list from the file
    inodeCount = fread(inodeList, sizeof(Inode), MAX_INODES, file);
    fclose(file);

    // Validate each inode entry
    size_t validInodes = 0;
    for (size_t i = 0; i < inodeCount; i++) {
        if (inodeList[i].inode >= MAX_INODES) {     // Check if inode number is valid
            printf("Warning: Invalid inode number %u. Ignoring entry.\n", inodeList[i].inode);
            continue;
        }
        if (inodeList[i].type != 'd' && inodeList[i].type != 'f') {  // Check if type is valid
            printf("Warning: Invalid type for inode %u. Ignoring entry.\n", inodeList[i].inode);
            continue;
        }
        inodeList[validInodes++] = inodeList[i]; // Keep only valid inodes
    }
    inodeCount = validInodes; // Update inode count to exclude invalid entries

    // Ensure that inode 0 exists and is a directory
    if (inodeCount == 0 || inodeList[0].inode != 0 || inodeList[0].type != 'd') {
        printf("Error: Root directory (inode 0) is missing or invalid.\n");
        exit(1);
    }

    // Set initial working directory
    currentInode = 0;
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
    
    // Create a temporary buffer to hold the truncated name
    char truncatedName[NAME_SIZE];
    strncpy(truncatedName, name, NAME_SIZE - 1);
    truncatedName[NAME_SIZE - 1] = '\0'; // Ensure null termination

    // Check if directory with truncated name already exists
    for (size_t i = 0; i < inodeCount; i++) {
        if (inodeList[i].parentInode == currentInode &&
            strcmp(inodeList[i].name, truncatedName) == 0) {
            printf("Directory already exists\n");
            return;
        }
    }

    // Create new directory inode with truncated name
    Inode newDir = {inodeCount, currentInode, 'd', ""};
    strncpy(newDir.name, truncatedName, NAME_SIZE - 1); // Copy name to inode
    newDir.name[NAME_SIZE - 1] = '\0'; // Ensure null termination
    inodeList[inodeCount++] = newDir;   // Add inode to inode list

    // Create an actual file for the directory
    char filename[16];
    snprintf(filename, sizeof(filename), "%u", newDir.inode);  // Create filename
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

    // Create a truncated name buffer
    char truncatedName[NAME_SIZE];
    strncpy(truncatedName, name, NAME_SIZE - 1);
    truncatedName[NAME_SIZE - 1] = '\0'; // Ensure null termination

    // Check if a file with the same truncated name already exists
    for (size_t i = 0; i < inodeCount; i++) {
        if (inodeList[i].parentInode == currentInode &&
            strcmp(inodeList[i].name, truncatedName) == 0) {
            printf("File already exists\n");
            return; // File already exists, do nothing
        }
    }

    // Create new file inode with truncated name
    Inode newFile = {inodeCount, currentInode, 'f', ""};   // Create inode
    strncpy(newFile.name, truncatedName, NAME_SIZE - 1);   // Copy name to inode  
    newFile.name[NAME_SIZE - 1] = '\0'; // Ensure null termination
    inodeList[inodeCount++] = newFile;   // Add inode to inode list

    // Create a file for the inode
    char filename[16];
    snprintf(filename, sizeof(filename), "%u", newFile.inode);  // Create filename
    FILE *file = fopen(filename, "w");
    if (file) {
        fprintf(file, "%s\n", truncatedName); // Store truncated name
        fclose(file);
    }
}


