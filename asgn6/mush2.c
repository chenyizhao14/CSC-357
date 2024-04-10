#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>

/* let's try this */
#include <errno.h>

#include "mush.h"

#define PROMPT "8-P "

#define READ_END 0
#define WRITE_END 1

extern int errno;

void print_prompt(FILE *src_file); 
void close_pipes(pipeline pipeline, int** allPipes);
/* ~pn-cs357/Given/Mush/libmush.*/

void handler(int signum) {
    /* blah */
}

int main(int argc, char *argv[]) {
    int i, j;
    int infile;
    int outfile;

    FILE* src_file;
    char* parsed_line; 
    struct pipeline* pipeline; 
    pid_t *children;

    int** allPipes;

    signal(SIGINT, handler);

    /* if no arguments, mush2 reads commands from stdin until eof (^D) */
    if(argc == 1) {
        src_file = stdin;

    } else if(argc == 2) {
        /* if 2 arguments, the file given on cmd line is the source file */
        if(!(src_file = fopen(argv[1], "r"))) {
            perror("fopen: src_file");
            exit(errno);
        }
    } else {
        fprintf(stderr, "usage: mush <infile file>\n");
        exit(EXIT_FAILURE);
    }

    /* steps to run mush2 
    1. parse the commandline 
    2. open file descriptors for stdin and outs of future children
        for file redirects use open or create the file
        for allPipes create pipe and connect ends using dup2
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
        /* readLongString returns NULL on EOF OR error.
         * in the former case, just continue and let the rest of 
         * my logic handle it. In the latter case, throw a fit. */
        parsed_line = readLongString(src_file);
        if(parsed_line == NULL) {
            if(feof(src_file)) {
                continue;
            } else {
                perror("readLongString");
                exit(errno);
            }
        }
        /* defined in parser which err it is */

        clerror = 0;
        pipeline = crack_pipeline(parsed_line);
        if(clerror) {
            /* Library will take care of printing the error,
             * we just have to make sure we don't try to run
             * a junk command. */
            print_prompt(src_file);
            continue;
        }

        free(parsed_line);

    /* typedef struct clstage *clstage;
        struct clstage {
        char *inname;       infile filename (or NULL for stdin) 
        char *outname;      outfile filename (NULL for stdout) 
        int argc;           argc and argv for the child 
        char **argv;        Array for argv 
    };

    typedef struct pipeline {
        char *cline;        the original command line 
        int length;         length of the pipeline 
        struct clstage *stage; descriptors for the stages 
    } *pipeline; */

        /* handle first the special case of cd */
        if(strcmp(pipeline->stage[0].argv[0], "cd") == 0) {
            /* can only cd if theres 1 path, 1 argument after cd */
            if(pipeline->stage[0].argc > 2) {
                fprintf(stderr, "usage: cd <directory>\n");
                /* in shell, don't exit */
            }
            else if(pipeline->stage[0].argc == 2) {
                /* if there is a path to chdir to */
                if(chdir(pipeline->stage[0].argv[1]) == -1) {
                    perror("chdir");
                }
            }
            else {
                /* just cd, return home */
                char *home;
            /* struct passwd : char* pw_dir is home directory */                

                if((home = getenv("HOME"))
                        || (home = (getpwuid(getuid())->pw_dir))) {
                    if(chdir(home) == -1) {
                        perror("chdir: home");
                        exit(errno);
                    }
                }
                else {
                    fprintf(stderr, "unable to determine home directory");
                    exit(EXIT_FAILURE);
                }
                    
            }
            print_prompt(src_file);
            continue;
        }
        
        /* when it is not cd */
        /* reset err number */
        errno = 0;
        children = malloc((pipeline->length) * sizeof(pid_t));
        if(errno) {
            perror("malloc");
            exit(errno);
        }

        /* makes sure that all children are not random number or 0 */
        memset(children, -1, pipeline->length);

        errno = 0;
        allPipes = (int**)calloc((pipeline->length) - 1, sizeof(int*));
        if(errno) {
            perror("calloc");
            exit(errno);
        }
        
        /* set up allPipes, so actually pipe it, pipe populates for you */
        for(i = 0; i < ((pipeline->length) - 1); i++) {
            allPipes[i] = (int*)calloc(2, sizeof(int));
            if(pipe(allPipes[i]) == -1) {
                perror("Couldn't pipe");
                exit(errno);
            } 
        }

        /* now that all pipes are ready, we can start forking */
        for(i = 0; i < (pipeline->length); i++) {
            if((children[i] = fork()) == -1) {
                perror("Couldn't fork");
                exit(errno);
            }
                /* if forking worked we increment the number of children */
                /* child_counter++; */
                
            /* in child */
            if(children[i] == 0) {

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
                
                /* close all pipes in children */
                close_pipes(pipeline, allPipes);

                /* finally we can execute and run */
                if (-1 == execvp(pipeline->stage[i].argv[0],
                    pipeline->stage[i].argv)){
                    /* returns only when failure*/
                    perror(pipeline->stage[i].argv[0]);
                }
            }
        }

        /* parent */
        /* don't forget to close pipes in parent too */
        close_pipes(pipeline, allPipes);

        /* Wait on all children with valid pids */
        for(j = 0; j < (pipeline->length); j++) {
            if(children[j] != -1 && waitpid(children[j], NULL, 0) == -1) {
                perror("wait");
                exit(errno);
            }
        }

        print_prompt(src_file);
        free(children);
        free(allPipes);
    }
   
    yylex_destroy(); 
    printf("\n");

    return 0;
}

void print_prompt(FILE *src_file) {
    if(isatty(fileno(src_file)) && isatty(fileno(stdout))) {
        printf(PROMPT);
        fflush(stdout);
    }
}

void close_pipes(pipeline pipeline, int** allPipes){
    int a, b;
        /* closes all our open pipes */
    for(a = 0; a < ((pipeline->length) - 1); a++) {
        for(b = 0; b < 2; b++) {
            if(allPipes[a][b]) {
                if(close(allPipes[a][b]) == -1) {
                    perror("Failed to close pipe.");
                    exit(errno);
                }
            }
        }
    }
}

