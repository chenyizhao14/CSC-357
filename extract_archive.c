#include <utime.h>

#include "create_archive.h"
#include "extract_archive.h"

/* What we need to do:
    first, after deciding that we need to extract, we need to read the header
    for each header we see, depending on typeflag
    1. typeflag directory
    2. typeflag symbolic link
    3. typeflag is a file
    we create directories and files 
    change the modes and permissions depending on the header */

/* PART 1 --------------------------------------------------
    the main of extraction, do things based on argc argv and flags */
int extract_main(int argc, char* argv[], int v_flag, int S_flag){
    FILE* archive;
    int success;
    Header* header;
    int i;

    /* the file name should be name_size + '/' + prefix_size + '\0 */
    char file_name[NAME_SIZE + PREFIX_SIZE + 2];

    archive = fopen(argv[2], "rb");
    memset(header, 0, BLOCK_SIZE);

    while (read_header(archive, header) != 0) {
        obtain_name(file_name, header->prefix, header->name);

        /* If no names are given on the command line,
         extracts all the files in the archive. */
        if (argc == 3) {
            success = extract_header(
                header, archive, file_name, v_flag, S_flag);
        } else {
            /* If a name or names are given on the command line,
                extract the given path and any and all descendents 
                of it just like listing. */
            for (i = 3; i < argc; ++i) {
                if (compare_file_names(file_name, argv[i]) == 0) {
                    /* if same, extract path*/
                    success = extract_header(
                    header, archive, file_name, v_flag, S_flag);
                }
            }
        }
    }
    fclose(archive);
    return success;
}

/* PART 2 ---------------------------------------------------
    based on the header and the typeflag, we go to file, syml, or dir */
int extract_header(
    Header* header, FILE* archive, char* file_name, int v_flag, int S_flag) {
    /* 0 on success, anything else on failure */
    int success = 0;

    if (v_flag) {
        printf("%s\n", file_name);
    }

    if (header->typeflag[0] == '0') {
        if (extract_file(file_name, archive, header) == 1) {
            perror("Error file extraction");
            success = 1;
        }
    } else if(header->typeflag[0] == '2') {
        if (extract_syml(header) == 1) {
            perror("Error syml extraction");
            success = 1;
        }
    } else if(header->typeflag[0] == '5') {
        if (extract_dir(header) == 1) {
            perror("Error directory extraction");
            success = 1;
        }
    } else {
        fprintf(stderr, "Invalid typeflag! '%c'", header->typeflag[0]);
        exit(EXIT_FAILURE);
    }
    return success;
}

    /* 
    switch((header->typeflag[0])) {
         extracts return 0 on success, 1 on failure
        case REG_FILE_TYPE:
            if (extract_file(file_name, archive, header) == 1) {
                perror("Error extraction");
                success = 1;
                break;
            }
        case SYM_LINK_TYPE:
            if (extract_syml(header) == 1) {
                perror("Error extraction");
                success = 1;
                break;
            }
        case DIRECTORY_TYPE:
            if (extract_dir(header) == 1) {
                perror("Error extraction");
                success = 1;
                break;
            }
        default: {
            fprintf(stderr, "Invalid typeflag! '%c'", header->typeflag[0]);
            exit(EXIT_FAILURE);
            }
        }
    return success;
} */


/* PART 3 ------------------------------------------
    extractions based on symlink, directory, and file */

/* if we extracted a symlink, set information */
int extract_syml(Header* header) {
    symlink(header->linkname, header->name);
    chmod(header->name, strtol(header->mode, NULL, 8));
    if (
        chown(header->name, 
            strtol(header->uid, NULL, 8), 
            strtol(header->gid, NULL, 8)) < 0
    ) {
        printf("Failed to change ownership of symlink.\n");
    }
    return 0;
}

/* if we extracted a directory, create directory and change ownerships */
int extract_dir(Header* header) {
    mkdir(header->name, strtol(header->mode, NULL, 8));
    if (chown(header->name, strtol(header->uid, NULL, 8), 
        strtol(header->gid, NULL, 8)) < 0) {
    }
    return 0;
}

/* if we extracted a file, create file and extract info from archive */
int extract_file(char* file_name, FILE* archive, Header* header) {
    /* read the archive file by blocks
    when read is successful, write to the output file
    reset the block to null and finish writing the last block*/
    struct utimbuf times;

    int buffer[BLOCK_SIZE] = {0};
    int file_size = strtol(header->size, NULL, 8);
    int current_size = 0;

    FILE* output_file;

    output_file = fopen(file_name, "wb");

    while(fread(buffer, BLOCK_SIZE, 1, archive)) {
        if (feof(archive)) {
            break;
        } else {
            if (current_size + BLOCK_SIZE > file_size) {
                break;
            } else {
                fwrite(buffer, BLOCK_SIZE, 1, output_file);
                current_size += BLOCK_SIZE;

                /* reset buffer to NULL */
                memset(buffer, 0, BLOCK_SIZE);
            }
        }
    }

    fwrite(buffer, (file_size - current_size), 1, output_file);
    fclose(output_file);

    times.modtime = strtol(header->mtime, NULL, 8);
    if(utime(file_name, &times)) {
        perror("Couldn't set utime");
        exit(1);
    }

    return 0;
}

void obtain_name(char* buffer, char* prefix, char* name) {
    /* if there is no prefix, the name is just the name */
    if (strlen(name) == 0) {
        buffer[0] = '\0';
    } else if (strlen(prefix) == 0) {
        strncpy(buffer, name, strlen(name));
        buffer[strlen(name)] = '\0';
    } else {
        /* if there is prefix, prefix '/' then name */
        strncpy(buffer, prefix, strlen(prefix));
        buffer[strlen(prefix)] = '/';
        strncpy((buffer + strlen(prefix) + 1), name, strlen(name));
        buffer[strlen(name) + strlen(prefix) + 1] = '\0';
    }
}

int read_header(FILE* archive, Header *header) {
    /* read 1 block of size 512 from the header and store into the header */
    int x = fread(header, BLOCK_SIZE, 1, archive);
    if (x == 0) {
        return 0;
    }
    return strtol(header->chksum, NULL, 8);
}

/* for the checking if file is the same as the one given in path */
int compare_file_names(char *a, char *b) {
    char a_buffer[strlen(b) + 1];
    char b_buffer[strlen(b) + 1];
    
    /* sets the two buffers to all '\0' */
    memset(a_buffer, '\0', strlen(b) + 1);
    memset(b_buffer, '\0', strlen(b) + 1);

    /* Copies the file name into the file buffer */
    strncpy(a_buffer, a, strlen(b));

    /* Copies the bd to name to the bd buffer */
    strcpy(b_buffer, b);

    /* return the bd result */
    return strcmp(a_buffer, b_buffer);
}
