#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define BLOCK_SIZE 512

typedef struct header {
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
} header;

int write_header(struct stat* header_stat, )

int main(int argc, char* argv[]) {

}