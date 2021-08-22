#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define RL_BUFFER 32768
char* mush_read_line(void) {

    unsigned int bufsize = RL_BUFFER;
    int position = 0;

    char* buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer) {
        fprintf(stderr, "[mush] allocation error\n");
        exit(EXIT_FAILURE);
    }

    while(1) {
        // Read a character
        c = getchar();

        // If we hit EOF, replace it with a null character and return

        if (c == EOF || c == '\n') {
            buffer[position] == '\0';
            return buffer;
        }

        else {
            buffer[position] = c;
        }

        position++;

        // If we have exceeded the buffer, reallocate

        if (position >= bufsize) {
            bufsize += RL_BUFFER;
            buffer = realloc(buffer, bufsize);

            if (!buffer) {
                fprintf(stderr, "[mush] allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

#define TOK_BUFSIZE 256
#define TOK_DELIM " \t\r\n\a"

char** mush_split_line(char* line) {
    int bufsize = TOK_BUFSIZE, position = 0;

    char** tokens = malloc(bufsize * sizeof(char*));
    char* token;

    if (!tokens) {
        fprintf(stderr, "[mush] allocation error");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));

            if (!tokens) {
                fprintf(stderr, "[mush] allocation error");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, TOK_DELIM);
    }

    tokens[position] = NULL;
    return tokens;
}

int mush_launch(char** args) {
    pid_t pid, wpid;

    int status;

    pid = fork();
    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("mush");
        }
        exit(EXIT_FAILURE);
    }

    else if (pid < 0) {
        // Error forking
        perror("mush");
    }

    else {
        // Parent process
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

// Function declarations for builtin shell commands
int mush_cd(char** args);
int mush_help(char** args);
int mush_exit(char** args);

// List of builtin commands
char* builtin_str[] = {
    "cd",
    "help",
    "exit"
};

int (*builtin_func[]) (char** ) = {
    &mush_cd,
    &mush_help,
    &mush_exit
};

int mush_num_builtins() {
    return sizeof(builtin_str) / sizeof(char* );
}

// Builtin function implementations

int mush_cd(char** args) {
    if (args[1] == NULL) {
        fprintf(stderr, "[mush] expected argument to `cd`\n");
    }

    else {
        if (chdir(args[1]) != 0) {
            perror("mush");
        }
    }

    return 1;
}

int mush_help(char** args) {
    printf("Monkelab's Mush\n");
    printf("Type stuff and hit return.\n");

    for (int i = 0; i < mush_num_builtins(); i++) {
        printf("\t%s\n", builtin_str[i]);
    }

    return 1;
}

int mush_exit(char** args) {
    return 0;
}

int mush_execute(char** args) {

    if (args[0] == NULL) {
        // An empty command was entered
        return 1;
    }

    for (int i = 0; i < mush_num_builtins(); i++) {
        if (!strcmp(args[0], builtin_str[i])) {
            return (*builtin_func[i])(args);
        }
    }

    return mush_launch(args);
}


void mush_loop(void) {
    char* line;
    char** args;
    int status;

    do {
        printf("> ");
        line = mush_read_line();
        args = mush_split_line(line);
        status = mush_execute(args);

        free(line);
        free(args);
    } while (status);
}


int main(int argc, char** argv) {
    // Load any config files

    // Run command loop
    mush_loop();

    // Shutdown/cleanup

    return EXIT_SUCCESS;
}
