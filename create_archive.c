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
    int linkname_len;
    struct stat* file_stat;
    char* file_name = file_entry -> d_name;
    struct passwd* u_name;
    struct group* g_name;
    int d_major;
    int d_minor;


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
        strncpy(header -> name, file_name, NAME_SIZE); /* put 1st 100 chars into name */
        for(i = 0; i < strlen(file_name) - NAME_SIZE; i++) { 
            /* put overflow into prefix char by char */
            (header -> prefix)[i] = file_name[NAME_SIZE + i];
        }
    }

    /*--------MODE----------*/
    sprintf(header -> mode, "%07o", file_stat -> st_mode); /* print mode into header */
    (header -> mode)[7] = "\0";

    /*-------UID AND GID ------*/
    sprintf(header -> uid, "%07o", file_stat -> st_uid);
    sprintf(header -> gid, "%07o", file_stat -> st_gid);
    (header -> uid)[7] = "\0";
    (header -> gid)[7] = "\0";

    /*-------SIZE------*/
    sprintf(header -> size, "%011o", file_stat -> st_size);

    /*-------MTIME------*/
    sprintf(header -> mtime, "%011o", file_stat -> st_mtime);
    (header -> mtime)[11] = "\0";

    /*------CHKSUM ------*/

    /*------FILETYPE, size for symlinks and dirs, linkname ------*/
    if(S_ISREG(file_stat -> st_mode)) {
        (header -> typeflag)[0] = REG_FILE_TYPE; /* typeflag = 0 */
    }
    else if(S_ISLNK(file_stat -> st_mode)) {
        (header -> typeflag)[0] = SYM_LINK_TYPE; /* typeflag = 2 */
        strcpy(header -> size, "000000000000"); /* size of symlinks is 0 */
        /* if symlink, set linkname to value of it*/
        linkname_len = readlink(file_name, header -> linkname, file_name, LINKNAME_SIZE);
        if(linkname_len < LINKNAME_SIZE) {
            (header -> linkname)[linkname_len - 1] = '\0'; /* add null terminator if there's space*/
        }
    }
    else if(S_ISDIR(file_stat -> st_mode)) {
        (header -> typeflag)[0] = DIRECTORY_TYPE; /* typeflag = 5 */
        strcpy(header -> size, "000000000000"); /* size of directory is 0 */
    }

    /*------MAGIC ------*/
    strcpy(header -> magic, "ustar");

    /*------VERSION ------*/
    strcpy(header -> version, "00");

    /*----------UNAME------------*/
    /* translate uid into name and gid into name*/
    u_name = getpwuid(file_stat -> st_uid);
    strcpy(header -> uname, u_name->pw_name);

    /*----------GNAME------------*/
    g_name = getgrgid(file_stat -> st_gid);
    strcpy(header -> gname, g_name->gr_name);

    /*----------DEVMAJOR------------*/
    d_major = major(file_stat->st_dev);
    sprintf(header->devmajor, "%08o", d_major);

    /*----------DEVMINOR------------*/
    d_minor = minor(file_stat->st_dev);
    sprintf(header->devminor, "%08o", d_minor);























}