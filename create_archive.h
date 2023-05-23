#ifndef CREATEARCHIVE_H
#define CREATEARCHIVE_H

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

#define NAME_SIZE 100
#define MODE_SIZE 8
#define UID_SIZE 8
#define GID_SIZE 8
#define SIZE_SIZE 12
#define MTIME_SIZE 12
#define CHKSUM_SIZE 8
#define TYPEFLAG_SIZE 1
#define LINKNAME_SIZE 100
#define MAGIC_SIZE 6
#define VERSION_SIZE 2
#define UNAME_SIZE 32
#define GNAME_SIZE 32
#define DEVMAJOR_SIZE 8
#define DEVMINOR_SIZE 8
#define PREFIX_SIZE 155
#define BLOCK_SIZE 512

#define REG_FILE_TYPE '0'
#define SYM_LINK_TYPE '2'
#define DIRECTORY_TYPE '5'

typedef struct __attribute__ ((__packed__)) file_header {
    char name[100]; /* NULL-terminated if NULL fits */
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag[1];
    char linkname[100]; /* NULL-terminated if NULL fits */
    char magic[6]; /* must be "ustar", NULL-terminated */
    char version[2];
    char uname[32]; /* NULL-terminated */
    char gname[32]; /* NULL-terminated */
    char devmajor[8];
    char devminor[8];
    char prefix[155]; /* NULL-terminated if NULL fits */
} Header;

Header* create_header(char* path_name);
char *append_name(char *directory_name, char *to_append);
void write_directory(char *directory_name, int outfile, int verbose);
// void write_entry(char *in_file_name, int outfile, int verbose);
void create_archive(char *file_path, int outfile, int verbose);
int calc_chksum(char* ptr);


#endif