#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/*
 * Built in shell commands
 */
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

char *builtin_str[] = {"cd", "help", "exit"};

int (*builtin_func[])(char **) = {&lsh_cd, &lsh_help, &lsh_exit};

int lsh_num_builtins() { return sizeof(builtin_str) / sizeof(char *); }

int lsh_cd(char **args) {
  if (args[1] == NULL) {
    fprintf(stderr, "lsh: cd: missing argument\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("lsh");
    }
  }
  return 1;
}

int lsh_help(char **args) {
  int i;
  printf("Simple Shell\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in\n");

  for (i = 0; i < lsh_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  return 1;
}

int lsh_exit(char **args) { return 0; }

/*
 * Pipping and redirection
 */
int lsh_pipe(char **left, char **right);

char *builtin_helpers[] = {"|", ">", "<"};

int (*helper_func[])(char **, char **) = {&lsh_pipe};

int lsh_num_helpers() { return sizeof(builtin_helpers) / sizeof(char *); }

/*
 * Pipping
 */
int lsh_pipe(char **left, char **right) {
  // Implement piping logic here
  // fork process, take the stdout from parent
  // and pass it into the child process which will exec
  // the command after the | symbol
  int pipefd[2];
  pid_t p1, p2;

  if (pipe(pipefd) == -1) {
    perror("lsh");
    return 1;
  }

  // Execute first command
  p1 = fork();
  if (p1 == 0) {
    dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to pipe
    close(pipefd[0]);
    close(pipefd[1]);

    if (execvp(left[0], left) == -1) {
      perror("lsh");
      exit(EXIT_FAILURE);
    }
  }

  // Execute second command
  p2 = fork();
  if (p2 == 0) {
    dup2(pipefd[0], STDIN_FILENO); // Redirect stdin to pipe
    close(pipefd[0]);
    close(pipefd[1]);

    if (execvp(right[0], right) == -1) {
      perror("lsh");
      exit(EXIT_FAILURE);
    }
  }

  // Close Parent
  close(pipefd[0]);
  close(pipefd[1]);

  waitpid(p1, NULL, 0); // Wait for first child
  waitpid(p2, NULL, 0); // Wait for second child

  return 1;
}

/*
 * Reads the line from the stream
 */
char *lsh_read_line(void) {
  int bufsize = 1024;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    // Read char
    c = getchar();

    if (c == EOF && position == 0) {
      exit(EXIT_SUCCESS);
    }

    // If EOF or newline, return buffer
    if (c == EOF || c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }

    position++;

    // If we have exceeded the buffer, reallocate buffer
    if (position >= bufsize) {
      bufsize += 1024;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

/*
 * Tokenizes the input line into an array of strings
 */
char **lsh_split_line(char *line) {
  int bufsize = 64, position = 0;
  char **tokens = malloc(bufsize * sizeof(char *));
  char *token;

  if (!tokens) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  // Tokenize the input with the given delimiters
  token = strtok(line, " \t\r\n\a");

  // While there are tokens, keep adding them to the array
  while (token != NULL) {
    tokens[position] = token;
    position++;

    // If we have exceeded the buffer, reallocate buffer
    if (position >= bufsize) {
      bufsize += 64;
      tokens = realloc(tokens, bufsize * sizeof(char *));
      if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, " \t\r\n\a");
  }
  tokens[position] = NULL;
  return tokens;
}

/*
 * Start the shell
 */
int lsh_launch(char **args) {
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    // We are child
    if (execvp(args[0], args) == -1) {
      perror("lsh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("lsh");
  } else {
    // We are parent
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

/*
 * Executes the given command in args
 */
int lsh_execute(char **args) {
  int i;

  if (args[0] == NULL) {
    // An empty command was entered
    return 1;
  }

  for (int i = 0; args[i] != NULL; i++) {
    if (strcmp(args[i], "|") == 0) {
      args[i] = NULL;
      return lsh_pipe(args, &args[i + 1]);
    }
  }

  for (i = 0; i < lsh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return lsh_launch(args);
}

/*
 * Main function loop that will read lines, parse the strings and execute the
 * commands Doesn't take in a function or return anything
 */
void lsh_loop(void) {
  char *line;
  char **args;
  int status;

  do {
    printf("> ");
    line = lsh_read_line();
    args = lsh_split_line(line);
    status = lsh_execute(args);

    free(line);
    free(args);
  } while (status);
}

int main(int argc, char **argv) {
  // Run Command Loop
  lsh_loop();

  return EXIT_SUCCESS;
}
