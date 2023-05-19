/* include <arpa/inet.h> */
#include <winsock2.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define BLOCK_SIZE 512

int main(int argc, char* argv[]) {
    char* file_name = argv[1];
    struct dirent* entry = 
    create_header();
}