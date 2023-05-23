#include "list_archive.h"
#include "create_archive.h"
#include "extract_archive.h"

/*For example:
% mytar tvf archive.tar
drwx------ pnico/pnico 0 2010-11-02 13:49 Testdir/
-rwx--x--x pnico/pnico 72 2010-11-02 13:49 Testdir/file1
-rw------- pnico/pnico 200 2010-11-02 13:49 Testdir/file2
% */

int print_list(Header* header, char* file_name, int v_flag) {
    char perms[10] = {0};
    int permissions = strtol(header->mode, NULL, 8);

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
    }
    return 0;
}

int list_archive(int argc, char* argv[], int v_flag) {
    FILE* archive;
    int success;
    Header* header;
    int i;

    /* the file name should be name_size + '/' + prefix_size + '\0 */
    char file_name[NAME_SIZE + PREFIX_SIZE + 2];

    archive = fopen(argv[2], "rb");
    memset(header, 0, BLOCK_SIZE);

    while (read_header(archive, header)) {
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
    }
    fclose(archive);
    return success;
}