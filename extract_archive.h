#ifndef EXTRACTARCHIVE_H
#define EXTRACTARCHIVE_H

#include "create_archive.h"

void obtain_name(char* buffer, char* prefix, char* name);
int read_header(FILE* archive, Header *header);
int compare_file_names(char *a, char *b);
int extract_main(int argc, char* argv[], int v_flag, int S_flag);
int extract_header(
    Header* header,
    FILE* archive,
    char* file_name, int v_flag, int S_flag);
int extract_syml(Header* header);
int extract_dir(Header* header);
/* int extract_file(); */

#endif