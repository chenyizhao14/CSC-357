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
#define BUFF_SIZE 4096

Header* create_header(char *file_name) {
    int i;
    int linkname_len;
    DIR* dir;
    struct dirent* entry; 
    struct stat file_stat;
    // char* file_name = file_entry -> d_name;
    struct passwd* u_name;
    struct group* g_name;
    int d_major;
    int d_minor;
    unsigned int checksum = 0;
    Header *header;
    unsigned char* ptr = (unsigned char*)&header;

    if ((dir = opendir(file_name)) == NULL) {
		fprintf(stderr, "cannot get current directory");
		exit(EXIT_FAILURE);
	}
    entry = readdir(dir);

    header = (Header *)malloc(sizeof(Header));
    if(header == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    if (lstat(entry -> d_name, &file_stat) == -1) { /* stat the file to get info*/
        perror("stat failed");
        exit(EXIT_FAILURE);
    }
    
    /*------NAME---------*/

    /* if file name fits in header name (100 bytes), put whole thing in there */
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
        strncpy(header -> name, file_name + (pointer + 1), NAME_SIZE); /* put last 100 chars into name */
        strncpy(header -> prefix, file_name, pointer); /* put chars before slash  */
    }

    /*--------MODE----------*/
    sprintf(header -> mode, "%08o", file_stat.st_mode); /* print mode into header */

    /*-------UID AND GID ------*/
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
        strcpy(header -> size, "000000000000"); /* size of symlinks is 0 */
        /* if symlink, set linkname to value of it*/
        linkname_len = readlink(file_name, header -> linkname, LINKNAME_SIZE);
        if(linkname_len < LINKNAME_SIZE) {
            (header -> linkname)[linkname_len - 1] = '\0'; /* add null terminator if there's space*/
        }
    }
    else if(S_ISDIR(file_stat.st_mode)) {
        (header -> typeflag)[0] = DIRECTORY_TYPE; /* typeflag = 5 */
        strcpy(header -> size, "000000000000"); /* size of directory is 0 */
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

    /*----------DEVMAJOR------------*/
    d_major = major(file_stat.st_dev);
    sprintf(header->devmajor, "%08o", d_major);

    /*----------DEVMINOR------------*/
    d_minor = minor(file_stat.st_dev);
    sprintf(header->devminor, "%08o", d_minor);

    /*------CHKSUM ------*/
    for (i = 0; i < BLOCK_SIZE; i++) {
        checksum += *ptr;
        ptr++;
    }

    /* Adds eight spaces */
    checksum += ' ' * CHKSUM_SIZE;
    sprintf(header->chksum, "%08o", checksum);

    return header;
}

void write_entry(char *in_file_name, int outfile) { /* outfile is the already open tar file*/
    /* writes the header and file contents if file*/
    int file;
    char buffer[1024]; /*file reading buffer*/
    ssize_t bytes_read, bytes_written;

    Header *header = create_header(in_file_name);
    
    if(header->typeflag[0] == REG_FILE_TYPE) {
        /* if file */
        int file;
        unsigned char padding[BLOCK_SIZE - sizeof(Header)] = {0};
        write(outfile, header, sizeof(Header)); /* write header */
        write(outfile, padding, BLOCK_SIZE - sizeof(Header)); /* write 12 extra bytes of padding to fill block of 512*/

        char buffer[BUFF_SIZE];

        /* open input file*/
        if(file = open(in_file_name, O_RDONLY) == - 1) {
            perror("can't opening file");
            exit(EXIT_FAILURE);
        }

        /* read from input file and write to tar file in chunks of 4096 */
        while(bytes_read = read(file, buffer, BUFF_SIZE) > 0) {
            bytes_written = write(outfile, buffer, BUFF_SIZE);
            if(bytes_written != bytes_read) {
                perror("error writing to file");
                exit(EXIT_FAILURE);
            }
        }

        if(bytes_read == -1) {
            perror("error readng from file");
            exit(EXIT_FAILURE);
        }

        close(file);
    }

    else if(header->typeflag[0] == SYM_LINK_TYPE) {
        /* if symlink */
        /*TODO: only write header */
        unsigned char padding[BLOCK_SIZE - sizeof(Header)] = {0};
        write(outfile, header, sizeof(Header)); /* write header */
        write(outfile, padding, BLOCK_SIZE - sizeof(Header)); /* write 12 extra bytes of padding to fill block of 512*/
    }
    else if(header->typeflag[0] == DIRECTORY_TYPE) {
        /* if directory */
        /*write header */
        unsigned char padding[BLOCK_SIZE - sizeof(Header)] = {0};
        write(outfile, header, sizeof(Header)); 
        write(outfile, padding, BLOCK_SIZE - sizeof(Header)); /* write 12 extra bytes of padding to fill block of 512*/
        /* write directory recursively*/
        write_directory(in_file_name, outfile); /* write everything in directory by looping through things in directory with readdir*/
    }
}

void write_directory(char *directory_name, int outfile) {
    DIR *curr_dir;
    struct stat file_stat;
    struct stat directory_stat;
    struct dirent *entry;
    char *file_name;
    Header *header = create_header(directory_name);

    if((curr_dir = opendir(directory_name)) != NULL) {
        stat(directory_name, &directory_stat); /* stat current directory*/
        while((entry = readdir(curr_dir)) != NULL) {
            if(entry->d_name[0] != '.' && entry->d_ino != directory_stat.st_ino) {
                /* check if file is parent directory*/
                file_name = append_name(directory_name, "/"); /* add file's name to path*/
                file_name = append_name(file_name, entry->d_name); /* add file's name to path*/
                /* stat the file*/
                if(lstat(file_name, &file_stat) == -1) {
                    perror("stat failed");
                    exit(EXIT_FAILURE);
                }
                /* recursive call*/
                write_entry(file_name, outfile);
            }
        }
    }

    closedir(curr_dir); /* close current directory */
    /* TODO: free it??? */
}

char *append_name(char *directory_name, char *to_append) {
    char *new_name;
    char *append;

    /* malloc new space for the new name with the correct size */
    new_name = malloc(sizeof(*new_name) + strlen(directory_name) + strlen(to_append) + 1);
    if(new_name == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    /* Copy the old name to new_name */
    strcpy(new_name, directory_name);
    /* Set the the next space after the old name to be a pointer to the rest of the name */
    append = new_name + strlen(directory_name);
    /* copy the new portion of the name to the pointer */
    strcpy(append, to_append);
    return new_name;
}

void create_archive(char *file_path, int outfile) {
    /* loops through all the entries in the path and writes them to the tar file */
    write_entry(file_path, outfile);
}

