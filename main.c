/*

ADD comments:
1. above function briefly describing it
2. also for each KEY line of code

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "task1.h"

int main(int argc, char *argv[]) 
{
    if (argc != 2) {
        printf("Usage: %s <filesystem_directory>\n", argv[0]);
        return 1;
    }

    const char *fsPath = argv[1];
    if (chdir(fsPath) != 0) {
        perror("chdir failed");
        return 1;
    }

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
