#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

char *builtin_str[] = {"cd", "help", "exit"};

int (*builtin_func[])(char **) = {&lsh_cd, &lsh_help, &lsh_exit};

int lsh_num_builtins() { return sizeof(builtin_str) / sizeof(char *); }

int lsh_cd(char **args) {
    if (args[1] == NULL || args[2] != NULL) {
        fprintf(stderr, "cd: expected argument is 1\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("lsh");
        }
    }
    return 1;
}

int lsh_help(char **args) {
    int i = 0;
    printf("Simple Shell\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in\n");
    for (i = 0; i < lsh_num_builtins(); i++) {
        printf("  %s\n", builtin_str[i]);
    }
    return 1;
}

int lsh_exit(char **args) {
    return 0;
}
