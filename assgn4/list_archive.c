#include <time.h>

#include "list_archive.h"
#include "create_archive.h"
#include "extract_archive.h"

#define PERMISSION_SIZE 10
#define OWNER_GROUP_NAME 17
#define SIZE 8
#define MTIME_PRINT_SIZE 16

/*For example:
% mytar tvf archive.tar
drwx------ pnico/pnico 0 2010-11-02 13:49 Testdir/
-rwx--x--x pnico/pnico 72 2010-11-02 13:49 Testdir/file1
-rw------- pnico/pnico 200 2010-11-02 13:49 Testdir/file2
% */

int print_list(Header* header, char* file_name, int v_flag) {
    char perms[PERMISSION_SIZE + 1] = {0};
    int permissions = strtol(header->mode, NULL, 8);
    char o_g_name[OWNER_GROUP_NAME];
    char* ptr = o_g_name;
    time_t mtime = strtol(header->mtime, NULL, 8);
    struct tm* time = NULL;
    char mtime_buffer[MTIME_PRINT_SIZE + 1];

    if (v_flag == 0) {
        printf("%s\n", file_name);
    } else if (v_flag) {
        if (header->typeflag[0] == REG_FILE_TYPE) {
            perms[0] = '-';
        } else if (header->typeflag[0] == SYM_LINK_TYPE) {
            perms[0] = '1';
        } else if (header->typeflag[0] == DIRECTORY_TYPE) {
            perms[0] = 'd';
        }

        if (permissions & S_IRUSR) {
            perms[1] = 'r';
        } else {
            perms[1] = '-';
        }
        if (permissions & S_IWUSR){
            perms[2] = 'w';
        } else {
            perms[2] = '-';
        }   
        if (permissions & S_IXUSR) {
            perms[3] = 'x';
        } else {
            perms[3] = '-';
        }
        if (permissions & S_IRGRP) {
            perms[4] = 'r';
        } else {
            perms[4] = '-';
        }
        if (permissions & S_IWGRP) {
            perms[5] = 'w';
        } else {
            perms[5] = '-';
        }
        if (permissions & S_IXGRP) {
            perms[6] = 'x';
        } else {
            perms[6] = '-';
        }
        if (permissions & S_IROTH) {
            perms[7] = 'r';
        } else {
            perms[7] = '-';
        }
        if (permissions & S_IWOTH) {
            perms[8] = 'w';
        } else {
            perms[8] = '-';
        }
        if (permissions & S_IXOTH) {
            perms[9] = 'x';
        } else {
            perms[9] = '-';
        }

    if (strlen(header->uname) > OWNER_GROUP_NAME) {
        strncpy(o_g_name, header->uname, OWNER_GROUP_NAME);
    } else {
        strncpy(o_g_name, header->uname, strlen(header->uname));
        ptr += strlen(header->uname);
        *ptr = '/';
        ptr++;

        if(strlen(header->uname) + strlen(header->gname) < OWNER_GROUP_NAME) {
            /* If the two names concatinated with a '/' 
            is less than 17 char, copy to buffer */
            strncpy(ptr, header->gname, strlen(header->gname));
        } else {
            /* If greater than 17, fill up as much of group name as possible */
            strncpy(o_g_name, header->gname, 
                OWNER_GROUP_NAME - strlen(header->gname));
        }    
        /* Get modifcation time for the file */
        time = localtime(&mtime);
        /* Put the times in a buffer */
        sprintf(mtime_buffer, "%04i-%02i-%02i %02i:%02i",
            1900 + (time->tm_year),
            time->tm_mon + 1,
            time->tm_mday,
            time->tm_hour,
            time->tm_min);

        /* Print all the info out */
        printf(
            "%10s %-17s %8i %16s %s\n",
            perms, o_g_name, strtol(header->size, NULL, 8), 
            mtime_buffer, file_name);
        }    
    }
    
    return 0;
}

int list_archive(int argc, char* argv[], int v_flag) {
    FILE* archive;
    int success;
    Header* header = (Header*)calloc(1, sizeof(Header));
    int i;
    int counter = 0;

    /* the file name should be name_size + '/' + prefix_size + '\0 */
    char file_name[NAME_SIZE + PREFIX_SIZE + 2];

    archive = fopen(argv[2], "rb");
    memset(header, 0, BLOCK_SIZE);

    while (read_header(archive, header) >= 0) {
        if(strlen(header->name) > 0) {
            obtain_name(file_name, header->prefix, header->name);

            if (argc == 3) {
                success = print_list(header, file_name, v_flag);
            } else {
                for (i = 3; i < argc; i++) {
                    /* Check if current file is the same as the file passed */
                    if (compare_file_names(file_name, argv[i]) == 0) {
                        /* If it is, print */
                        success = print_list(header, file_name, v_flag);
                    }
                }
            }
            int size = strtol(header->size, NULL, 8);
            int distance;
            if(size) {
                distance = size / BLOCK_SIZE + 1;
            }
            else {
                distance = 0;
            }
            // int distance = size ? size // BLOCK_SIZE + 1/: 0;
            if(fseek(archive, distance * BLOCK_SIZE, SEEK_CUR) != 0) {
                perror("fseek failed");
                exit(EXIT_FAILURE);
            }
        } else {
            break;
        }

    }
    fclose(archive);
    return success;
}
