#include <stdio.h>
#include <string.h>

#include "create.h"
#include "fat32.h"
#include "lexer.h"
#include "navigation.h"

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [FAT32 IMAGE]\n", argv[0]);
        return 1;
    }

    FileSystem fs;
    if (mount_image(&fs, argv[1]) != 0)
        return 1;

    while (1) {
        printf("%s%s> ", fs.image_name, fs.cwd_path);
        fflush(stdout);

        char *input = get_input();
        if (input == NULL)      /* Ctrl-D / end of input */
            break;

        tokenlist *tokens = get_tokens(input);
        free(input);

        if (tokens->size == 0) {
            free_tokens(tokens);
            continue;
        }

        char *cmd = tokens->items[0];

        if (strcmp(cmd, "exit") == 0) {
            free_tokens(tokens);
            break;
        } else if (strcmp(cmd, "info") == 0) {
            if (tokens->size != 1)
                printf("Error: info takes no arguments\n");
            else
                cmd_info(&fs);
        } else if (strcmp(cmd, "ls") == 0) {
            if (tokens->size != 1)
                printf("Error: ls takes no arguments\n");
            else
                cmd_ls(&fs);
        } else if (strcmp(cmd, "cd") == 0) {
            if (tokens->size != 2)
                printf("Error: usage: cd [DIRNAME]\n");
            else
                cmd_cd(&fs, tokens->items[1]);
        } else if (strcmp(cmd, "mkdir") == 0) {
            if (tokens->size != 2)
                printf("Error: usage: mkdir [DIRNAME]\n");
            else
                cmd_mkdir(&fs, tokens->items[1]);
        } else if (strcmp(cmd, "creat") == 0) {
            if (tokens->size != 2)
                printf("Error: usage: creat [FILENAME]\n");
            else
                cmd_creat(&fs, tokens->items[1]);
        } else {
            printf("Error: unknown command '%s'\n", cmd);
        }

        free_tokens(tokens);
    }

    unmount_image(&fs);
    return 0;
}
