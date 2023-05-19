#include "create_archive.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>

Header* create_header(struct dirent* file_entry) {
    int i;
    struct stat* file_stat;
    char* file_name = file_entry -> d_name;

    Header *header = (Header *)malloc(sizeof(Header));
    if(header == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    stat(file_name, file_stat); /* stat the file to get info*/
    
    /*------NAME---------*/

    /* if file name fits in header name (100 bytes), put whole thing in there */
    if(strlen(file_name) < NAME_SIZE) {
        strcpy(header -> name, file_name); 
    }

    /* if file name doesn't fit in header name, put overflow into prefix*/
    if(strlen(file_name) > NAME_SIZE) {
        strncpy(header -> name, file_name, NAME_SIZE) /* put 1st 100 chars into name */
        for(i = 0; i < strlen(file_name) - NAME_SIZE; i++) { 
            /* put overflow into prefix char by char */
            (header -> prefix)[i] = file_name[NAME_SIZE + i];
        }
    }

    /*--------MODE----------*/
    



}