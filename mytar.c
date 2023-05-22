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

int main(int argc, char* argv[]) {

    // Header* header;
    int outfile;
    char* file_name = argv[1];

    // if(argv[1] == 'cf') {

    // }
    
    if(argv[1] != NULL) {
        outfile = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 
        S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    }

    create_archive(file_name, outfile);

    return 0;
}
