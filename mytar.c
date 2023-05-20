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

#include "create_archive.h"

int main(int argc, char* argv[]) {
    DIR* dir;
    struct dirent* entry; 
    Header* header;
    char* file_name = argv[1];
    if ((dir = opendir(file_name)) == NULL) {
		fprintf(stderr, "cannot get current directory");
		exit(EXIT_FAILURE);
	}
    entry = readdir(dir);
    header = create_header(entry);
    return 0;
}
