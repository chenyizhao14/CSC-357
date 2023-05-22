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

void obtain_name(char* buffer, char* prefix, char* name) {
    /* if there is no prefix, the name is just the name */
    if (strlen(prefix) == 0) {
        strncpy(buffer, name, strlen(name));
    } else {
        /* if there is prefix, prefix '/' then name */
        strncpy(buffer, prefix, strlen(prefix));
        buffer[strlen(prefix)] = '/';
        strncpy((buffer + strlen(prefix) + 1), name, strlen(name));
    }
}

int read_header(FILE* archive, Header *header) {
    /* read 1 block of size 512 from the header and store into the header */
    fread(header, BLOCK_SIZE, 1, archive);
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

/* PART 1 --------------------------------------------------
    the main of extraction, do things based on argc argv and flags */
int extract_main(int argc, char* argv[], int v_flag, int S_flag){
    FILE* archive;
    int success;
    Header *header = {0};
    int i;

    /* the file name should be name_size + '/' + prefix_size + '\0 */
    char file_name[NAME_SIZE + PREFIX_SIZE + 2];

    archive = fopen(argv[2], "rb");

    while (read_header(archive, header)) {
        obtain_name(file_name, header->prefix, header->name);

        /* If no names are given on the command line,
         extracts all the files in the archive. */
        if (argc == 3) {
            success = extract_header(header, archive, file_name, v_flag, S_flag);
        } else {
            /* If a name or names are given on the command line,
                extract the given path and any and all descendents 
                of it just like listing. */
            for (i = 3; i < argc; ++i) {
                if (compare_file_names(file_name, argv[i]) == 0) {
                    /* if same, extract path*/
                    success = extract_header(header, archive, file_name, v_flag, S_flag);

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

    switch(strtol(header->typeflag, NULL, 8)) {
        /* extracts return 0 on success, 1 on failure*/
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
            fprintf(stderr, "Invalid typeflag! '%c'", header->typeflag);
            exit(EXIT_FAILURE);
            }
        }
    return success;
}


/* PART 3 ------------------------------------------
    extractions based on symlink, directory, and file */

/* if we extracted a symlink, set information */
int extract_syml(Header* header) {
    symlink(header->linkname, header->name);
    chmod(header->name, strtol(header->mode, NULL, 8));
    if (chown(header->name, strtol(header->uid, NULL, 8), strtol(header->gid, NULL, 8)) < 0) {
        printf("Failed to change ownership of symlink.\n");
        perror("chown");      
        return 1;
    }
    return 0;
}

/* if we extracted a directory, create directory and change ownerships */
int extract_dir(Header* header) {
    mkdir(header->name, strtol(header->mode, NULL, 8));
    if (chown(header->name, strtol(header->uid, NULL, 8), strtol(header->gid, NULL, 8)) < 0) {
        printf("Failed to change ownership of directory.\n");
        perror("chown");   
        return 1;
    }
    return 0;
}

/* if we extracted a file, create file and extract info from archive */
int extract_file(char* file_name, FILE* archive, Header* header) {
    /* read the archive file by blocks
    when read is successful, write to the output file
    reset the block to null and finish writing the last block*/

    int buffer[BLOCK_SIZE];
    int file_size = strtol(header->size, NULL, 8);
    int current_size = 0;

    FILE* output_file;

    output_file = fopen(file_name, "wb");

    while(fread(buffer, BLOCK_SIZE, 1, archive)) {
        if (feof(archive)) {
            break;
        } else {
            fwrite(buffer, BLOCK_SIZE, 1, output_file);
            current_size += BLOCK_SIZE;

            /* reset buffer to NULL */
            memset(buffer, 0, BLOCK_SIZE);
        }
    }

    fwrite(buffer, file_size - current_size, 1, output_file);
    fclose(output_file);

    if (chmod(header->name, strtol(header->mode, NULL, 8)) < 0) {
        printf("Failed to change mode of file.\n");
        perror("chmod");  
        return 1;
    }

    if (
        chown(header->name, 
        strtol(header->uid, NULL, 8), 
        strtol(header->gid, NULL, 8)) < 0) 
    {
        printf("Failed to change ownership of file.\n");
        perror("chown");  
        return 1;
    }

    return 0;
}
