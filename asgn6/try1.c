#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <pwd.h>

#include "mush.h"

#define READ_END 0
#define WRITE_END 1
#define PROMPT "8-P "

void print_prompt(FILE* file);
void closePipes(pipeline pipeline, int** fds);

/* ~pn-cs357/Given/Mush/libmush.*/

void handler(int signum) {
    /* blah */
}

int main(int argc, char* argv[]) {

    FILE* src_file;
    char* parsed_line;
    pipeline pipeline;
    int infile;
    int outfile;

    signal(SIGINT, handler);

    /* if no arguments, mush2 reads commands from stdin until eof (^D) */
    if (argc == 1) {
        src_file = stdin;

    } else if (argc == 2) {
        /* if 2 arguments, the file given on cmd line is the source file */
        if(!(src_file = fopen(argv[1], "r"))) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "usage: mush [<input file>]");
        exit(EXIT_FAILURE);
    }

    /* steps to run mush2 
    1. parse the commandline 
    2. open file descriptors for stdin and outs of future children
        for file redirects use open or create the file
        for pipes create pipe and connect ends using dup2
        if no redirection leave child's stdin or stdout same as parent
    3. cd is a special case, shell executes itself rather than child
    4. launch children, for each command for a child process
        when child process begins executing, dup appropriate fds to stdin,out
        close all other fds and exec the command
    5. after children are launched, parent waits for all to terminate
        know how many children you have and count properly
    6. after all children die, shell resets itself, flush stdout, print prompt
    
    Notes: print prompts iff both stdin and stdout are ttys (useisatty)
    Using the parser:
        1. read a command line(readlongstring()) 
        2. pass it to crack_pipeline, which returns struct pipeline
            contains original commandline, length, and array of structs
      */
    
    print_prompt(src_file);
    
    while(!feof(src_file)) {
        /* read long string is malloced */
        parsed_line = readLongString(src_file);

        /* if any time of failure we continue */
        if (parsed_line == NULL) {
            if (feof(src_file)) {
                continue;
            } else {
                perror("readLongString");
                exit(2);
            }
        }

        pipeline = crack_pipeline(parsed_line);

        free(parsed_line);

/* typedef struct clstage *clstage;
struct clstage {
char *inname;       input filename (or NULL for stdin) 
char *outname;      output filename (NULL for stdout) 
int argc;           argc and argv for the child 
char **argv;        Array for argv 
};

typedef struct pipeline {
char *cline;        the original command line 
int length;         length of the pipeline 
struct clstage *stage; descriptors for the stages 
} *pipeline; */

        /* handle first the special case of cd */
        if (strcmp(pipeline->stage->argv[0], "cd") == 0) {
            /* can only cd if theres 1 path, 1 argument after cd */
            if (pipeline->stage->argc > 2) {
                fprintf(stderr, "usage cd <directory>\n");
                /* in shell, don't exit */
            } else if (pipeline->stage->argc == 2) {
                /* if there is a path to chdir to */
                if(chdir(pipeline->stage->argv[1]) == -1) {
                    perror("chdir");
                }
            } 
            else {
                /* just cd, return home */
                char* home;
                if ((home = getenv("HOME")) || 
                    (home = (getpwuid(getuid())->pw_dir))) {
                    if (chdir(home) == -1) {
                        perror("chdir: home");
                        exit(EXIT_FAILURE);
                    }
                } 
                else {
                    fprintf(stderr, "unable to determine home directory");
                    continue;
                }
                    /* struct passwd : char* pw_dir is home directory */                
            }
        }

        /* when it is not cd */
        int i;
        /* pid_t child; */
        /* can't seem to figure out where to count children */
        /* so lets try this instead */
        pid_t* children;
        /* set up pipes */
        /* we can have **pipes with number of pipes
            and then each number has its own fd[2] */
        int **allPipes = (int**)calloc(
            (pipeline->length) - 1, sizeof(int*));

        if (allPipes == NULL) {
            perror("calloc");
            break;
        }

        for (i = 0; i < (pipeline->length) - 1; i++) {
            allPipes[i] = (int*)calloc(2, sizeof(int));
            if (allPipes[i] == NULL) {
                perror("calloc");
                break;
            }
            /* set up pipes, so actually pipe it, pipe populates for you */
            if (pipe(allPipes[i]) == -1) {
                perror("pipe");
                break;
            }
        }

        children = malloc((pipeline->length) * sizeof(pid_t));
        memset(children, -1, pipeline->length); /* so its not 0 */

        /* now that all pipes are ready, we can start forking */
        for (i = 0; i < (pipeline->length); i++) {
            /* in the case that */
            if((children[i] == fork()) == -1){
                perror("fork");
                break;
            }
                /* if forking worked we increment the number of children */
                /* child_counter++; */
                
            /* in the child */
            if (children[i] == 0) {

                if(pipeline->stage[i].inname != NULL) {
                    infile = open(pipeline->stage[i].inname, O_RDONLY);
                    if (infile == -1) {
                        perror("open: infd");
                        exit(1);
                    }
                    if (dup2(infile, STDIN_FILENO) == -1) {
                        perror("dup2");
                        exit(1);
                    }
                    close(infile);
                } else {
                    /* take from stdin if inname is null 
                    don't overwrite stdin if and only if its the first */
                    if (i != 0) {
                        if(dup2(allPipes[i-1][READ_END], STDIN_FILENO) == -1) {
                            perror("dup2");
                            exit(EXIT_FAILURE);
                        }
                    }
                }

                if(pipeline->stage[i].outname != NULL) {
                    /* open the given file, if it exists truncate
                    if it doesn't exist create it with read and write */
                    outfile = open(pipeline->stage[i].outname,
                        O_WRONLY | O_CREAT | O_TRUNC,
                        S_IRUSR | S_IWUSR | S_IRGRP |
                        S_IWGRP | S_IROTH | S_IWOTH);

                    if (outfile == -1) {
                        perror("open: outfile");
                        exit(2);
                    }

                    if (dup2(outfile, STDOUT_FILENO) == -1) {
                        perror("dup2");
                        exit(2);
                    }
                    close(outfile);
                } else {
                /* so outname is null */
                    if(i != (pipeline->length -1)){
                    /* if its not the last stage */
                        if(dup2(allPipes[i][WRITE_END], STDOUT_FILENO) == -1) {
                            perror("dup2");
                            break;   
                        }
                    }                     
                }

                closePipes(pipeline, allPipes);

                /* finally we can execute and run */
                if (-1 == execvp(pipeline->stage[i].argv[0],
                    pipeline->stage[i].argv)){
                    /* returns only when failure*/
                    perror(pipeline->stage[i].argv[0]);
                }
            }
        }

        /* parent */

        closePipes(pipeline, allPipes);
        for (i = 0; i < (pipeline->length) - 1; i++) {
            free(allPipes[i]);
        }
            /* Wait on all valid PIDs */
        for(i = 0; i < (pipeline->length) - 1; i++) {
            if(children[i] != -1 && waitpid(children[i], NULL, 0) == -1) {
                perror("Failed to wait on child.");
                exit(EXIT_FAILURE);
            }
        }

        print_prompt(src_file);
        free(allPipes);
        free_pipeline(pipeline);
    }

    yylex_destroy();
    return 0;
}

void closePipes(pipeline pipeline, int** allPipes) {
    int a;
    int b;
        /* close all pipes now that we're done: parent too*/
    for(a = 0; a < ((pipeline->length) - 1); a++) {
    /* close all pipes, read and write ends */
        for(b = 0; b < 2; b++) {
            if(allPipes[a][b]) {
                if(close(allPipes[a][b]) == -1) {
                    perror("Failed to close pipe.");
                    break;
                }
            }
        }
    }
}

void print_prompt(FILE* file) {
    if(isatty(fileno(file)) && isatty(fileno(stdout))) {
        printf(PROMPT);
        fflush(stdout);
    }
}

/* create a pipe, stdin tty, stdout to the pipe
second child stdin from the pipe, stdout to tty*/