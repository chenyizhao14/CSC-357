#include <arpa/inet.h>
/* #include <winsock2.h> */
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <fcntl.h>

#include "create_archive.h"
#include "extract_archive.h"
#include "list_archive.h"

int main(int argc, char* argv[]) {

    // Header* header;
    int outfile;
    int i;
    char* file_name = argv[2];
    int x = 0;
    int c = 0;
    int t = 0;
    int f = 0;
    int v_flag = 0;
    int s_flag = 0;
    char* options = argv[1];
    
/*
    // if(argv[1] == 'cf') {

    // }

    if(argv[1] != NULL) {
        outfile = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 
        S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    }

    create_archive(file_name, outfile);
    */

    if (argc == 0) {
        fprintf(stderr, "No arguments\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < strlen(options); i++) {
        if (options[i] == 'x') {
            x = 1;
        } else if (options[i] == 'c') {
            c = 1;
        } else if (options[i] == 't') {
            t = 1;
        }

        if (options[i] == 'v') {
            v_flag = 1;
        }
        if (options[i] == 'S') {
            s_flag = 1;
        }
        if (options[i] == 'f') {
            f = 1;
        }
    }

    if (!(x) && !(c) && !(t)) {
        fprintf(stderr, "invalid 2nd argument \n");
    }
    if (!(f)) {
        fprintf(stderr, "Missing f in 2nd argument\n");
        exit(EXIT_FAILURE);
    }

    if (x){
        extract_main(argc, argv, v_flag, s_flag);
    }

    if (c) {
        file_name = argv[3];
        
        if(argv[2] == NULL) {
            fprintf(stderr, "no tarfile");
            exit(EXIT_FAILURE);
        } else if (argv[2] != NULL){
            outfile = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 
            S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        }
        
        create_archive(file_name, outfile, v_flag);

    }

    if (t) {
        if (!strstr(argv[2], ".tar")) {
            fprintf(stderr, "Passed file is not a .tar\n");
            exit(EXIT_FAILURE);
        }
        list_archive(argc, argv, v_flag);
    }

    return 0;
}