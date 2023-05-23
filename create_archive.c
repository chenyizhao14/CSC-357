#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <fcntl.h>

#include "create_archive.h"
#include "special_int.h"

#define BUFF_SIZE 4096

Header* create_header(char *file_name) {
    int i;
    int linkname_len;
    DIR* dir;
    struct dirent* entry; 
    struct stat file_stat;
    struct passwd* u_name;
    struct group* g_name;
    int d_major;
    int d_minor;
    unsigned int checksum = 0;
    Header *header;
    unsigned char* ptr = (unsigned char*)&header;

    header = (Header *)malloc(sizeof(Header));
    if(header == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    if (lstat(/*entry -> d_name*/ file_name, &file_stat) == -1) { 
        /* stat the file to get info*/
        perror("stat failed");
        exit(EXIT_FAILURE);
    }
    
    /*------NAME---------*/

    /* if file name fits in header name (100 bytes),
     put whole thing in there */
    if(strlen(file_name) < NAME_SIZE) {
        strcpy(header -> name, file_name); 
    }

    /* if file name doesn't fit in header name, put overflow into prefix*/
    if(strlen(file_name) > NAME_SIZE) {
        int start = strlen(file_name) - 1 - NAME_SIZE;
        int pointer = start;
        /* start looping through name from 100 from the end */
        for(i = start; i < strlen(file_name); i++) {
            if(file_name[i] == '/') {
                pointer = i; /* index of the slash*/
            }
        }
        /* put last 100 chars into name */
        strncpy(header -> name, file_name + (pointer + 1), NAME_SIZE); 
        /* put chars before slash into prefix */
        strncpy(header -> prefix, file_name, pointer); 
    }

    /*--------MODE----------*/
    /* print mode into header */
    sprintf(header -> mode, "%08o", file_stat.st_mode & 07777); 

    /*-------UID AND GID ------*/
    // int uid_len;
    // char uid[8];
    // sprintf(uid, "%o", file_stat.st_uid);
    // if ((uid_len = strlen(uid)) < 8) {
    //     sprintf(header -> uid, "%08o", file_stat.st_uid);
    // } 
    // else {
    //     /* If uid_len > 7, compress it and add to header */
    //     insert_special_int(header->uid, 8, file_stat.st_uid);
    // }
    
    sprintf(header -> uid, "%08o", file_stat.st_uid);
    sprintf(header -> gid, "%08o", file_stat.st_gid);

    /*-------SIZE------*/
    sprintf(header -> size, "%011o", file_stat.st_size);

    /*-------MTIME------*/
    sprintf(header -> mtime, "%012o", file_stat.st_mtime);

    /*------FILETYPE, size for symlinks and dirs, linkname ------*/
    if(S_ISREG(file_stat.st_mode)) {
        (header -> typeflag)[0] = REG_FILE_TYPE; /* typeflag = 0 */
    }
    else if(S_ISLNK(file_stat.st_mode)) {
        (header -> typeflag)[0] = SYM_LINK_TYPE; /* typeflag = 2 */
        strcpy(header -> size, "00000000000"); /* size of symlinks is 0 */
        /* if symlink, set linkname to value of it*/
        linkname_len = 
            readlink(file_name, header -> linkname, LINKNAME_SIZE);
        if(linkname_len < LINKNAME_SIZE) {
            /* add null terminator if there's space*/
            (header -> linkname)[linkname_len - 1] = '\0'; 
        }
    }
    else if(S_ISDIR(file_stat.st_mode)) {
        (header -> typeflag)[0] = DIRECTORY_TYPE; /* typeflag = 5 */
        strcpy(header -> size, "00000000000"); /* size of directory is 0 */
    }

    /*------MAGIC ------*/
    strcpy(header -> magic, "ustar");

    /*------VERSION ------*/
    strcpy(header -> version, "00");

    /*----------UNAME------------*/
    /* translate uid into name and gid into name*/
    u_name = getpwuid(file_stat.st_uid);
    strcpy(header -> uname, u_name->pw_name);

    /*----------GNAME------------*/
    g_name = getgrgid(file_stat.st_gid);
    strcpy(header -> gname, g_name->gr_name);

    // /*----------DEVMAJOR------------*/
    // d_major = major(file_stat.st_dev);
    // sprintf(header->devmajor, "%08o", d_major);
    sprintf(header->devmajor, "\0\0\0\0\0\0\0", d_major);

    // /*----------DEVMINOR------------*/
    // d_minor = minor(file_stat.st_dev);
    // sprintf(header->devminor, "%08o", d_minor);
    sprintf(header->devminor, "\0\0\0\0\0\0\0", d_minor);

    /*------CHKSUM ------*/
    for (i = 0; i < BLOCK_SIZE; i++) {
        checksum += *ptr;
        ptr++;
    }

    /* Adds eight spaces */
    checksum += ' ' * CHKSUM_SIZE;
    sprintf(header->chksum, "%07o", checksum);

    return header;
}

void create_archive(char *in_file_name, int outfile, int verbose) { 
    /* loops through all the entries in the path 
    and writes them to the tar file */

    /* outfile is the already open tar file*/

    /* writes the header and file contents if file*/
    int file;
    char buffer[1024] = {0}; /*file reading buffer*/
    int bytes_read = 0;
    int bytes_written = 0;

    Header *header = create_header(in_file_name);
    
    if(header->typeflag[0] == REG_FILE_TYPE) {
        /* if file */
        if(verbose == 1) {
            printf("%s\n", in_file_name);
            // printf("%s:file contents\n", in_file_name);
        }
        // int file;
        unsigned char padding[BLOCK_SIZE - sizeof(Header)] = {0};
        write(outfile, header, sizeof(Header)); /* write header */
        /* write 12 extra bytes of padding to fill block of 512*/
        write(outfile, padding, BLOCK_SIZE - sizeof(Header)); 
        char buffer[BUFF_SIZE];

        /* open input file*/
        file = open(in_file_name, O_RDONLY);
        if(file == - 1) {
            perror("can't opening file");
            exit(EXIT_FAILURE);
        }

        /* read from input file and write to tar file in chunks of 4096 */
        while((bytes_read = read(file, buffer, BUFF_SIZE)) > 0) {
            bytes_written = write(outfile, buffer, bytes_read);
            memset(buffer, 0, BUFF_SIZE);
            // // if(bytes_written != bytes_read) {
            // //     perror("error writing to file");
            // //     exit(EXIT_FAILURE);
            // // }
        }

        if(bytes_read == -1) {
            perror("error reading from file");
            exit(EXIT_FAILURE);
        }

        close(file);
    }

    else if(header->typeflag[0] == SYM_LINK_TYPE) {
        /* if symlink */
        if(verbose == 1) {
            printf("%s/\n", in_file_name);
        }
        unsigned char padding[BLOCK_SIZE - sizeof(Header)] = {0};
        write(outfile, header, sizeof(Header)); /* write header */
        /* write 12 extra bytes of padding to fill block of 512*/
        write(outfile, padding, BLOCK_SIZE - sizeof(Header)); 
    }
    else if(header->typeflag[0] == DIRECTORY_TYPE) {
        /* if directory */
        /*write header */
        if(verbose == 1) {
                    printf("%s/\n", in_file_name);
        }
        unsigned char padding[BLOCK_SIZE - sizeof(Header)] = {0};
        write(outfile, header, sizeof(Header)); 
        /* write 12 extra bytes of padding to fill block of 512*/
        write(outfile, padding, BLOCK_SIZE - sizeof(Header)); 
        // /* write directory recursively*/
        /* write everything in directory by looping 
        through things in directory with readdir*/
        write_directory(in_file_name, outfile, verbose); 
    }
}

void write_directory(char *directory_name, int outfile, int verbose) {
    DIR *curr_dir;
    struct stat file_stat;
    struct stat directory_stat;
    struct dirent *entry;
    char *file_name;
    Header *header = create_header(directory_name);
    curr_dir = opendir(directory_name);
    if(curr_dir == NULL) {
        perror("cannot open directory");
        return;
    }

    stat(directory_name, &directory_stat); /* stat current directory*/

    while((entry = readdir(curr_dir)) != NULL) { 
        /* read all entries in directory */
        if(entry->d_name[0] != '.' && entry->d_ino != directory_stat.st_ino) {
            /* check if file is parent directory*/

            /* add file's name to path*/
            file_name = append_name(directory_name, "/"); 
            /* add file's name to path*/
            file_name = append_name(file_name, entry->d_name); 

            /* stat the file*/
            if(lstat(file_name, &file_stat) == -1) {
                perror("stat failed");
                exit(EXIT_FAILURE);
            }
            /* recursively call write directory call*/
            create_archive(file_name, outfile, verbose);
        }
    }
    closedir(curr_dir); /* close current directory */
    /* TODO: free it??? */
}

char *append_name(char *directory_name, char *to_append) {
    char *new_name;
    char *append;

    /* malloc new space for the new name with the correct size */
    new_name = malloc(sizeof(*new_name) + 
        strlen(directory_name) + strlen(to_append) + 1);
    if(new_name == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    /* copy old name to new_name */
    strcpy(new_name, directory_name);
    /* set next space after  old name to 
    point to rest of  name */
    append = new_name + strlen(directory_name);
    /* copy new part of name */
    strcpy(append, to_append);
    return new_name;
}

// void create_archive(char *file_path, int outfile, int verbose) {
//     /* loops through all the entries in the path 
//     and writes them to the tar file */
//     write_entry(file_path, outfile, verbose);
// }

