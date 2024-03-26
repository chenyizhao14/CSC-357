#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <winsock2.h>
#include <string.h>

#include "huffman.h"

#define MAX_BIT_SHIFT 7

int read_freq_table(int input, FreqTable* freq_table) {
    unsigned char header;
    int i;
    unsigned char value;
    unsigned count;

    init_freq_table(freq_table);

    /* read the number of entries */
    if (read(input, &header, sizeof(header)) != sizeof(header)) {
        /* no frequencies */
        return 0;
    }

    /* read all frequency entries */
    for (i = 0; i <= header; ++i) {
        if (read(input, &value, sizeof(value)) != sizeof(value)) {
            perror("Unable to read the input file");
            return 1;
        }

        if (read(input, &count, sizeof(count)) != sizeof(count)) {
            perror("Unable to read the input file");
            return 1;
        }

        freq_table->table[value] = ntohl(count);
    }

    return 0;
}

int decode_file(int input, int output, FreqTable* freq_table, Node* root) {
    unsigned output_size = 0;
    unsigned written_size;
    int i;
    Node* cur_node;
    unsigned char byte;
    unsigned bit_count;

    /* count output file size */
    for (
        i = 0; 
        i < sizeof(freq_table->table) / sizeof(*(freq_table->table));
        ++i
    ) {
        output_size += freq_table->table[i];
    }

    /* read the input file */
    cur_node = root;
    bit_count = 0;

    for (written_size = 0; written_size < output_size;) {
        /* if we reach leaf node, write character to the output */
        if (!(cur_node->left)) {
            if (write(
                output,
                &(cur_node->value),
                sizeof(cur_node->value)
            ) != sizeof(cur_node->value)) {
                perror("Unable to write to the output file");
                return 1;
            }

            cur_node = root;
            ++written_size;

            continue;
        }

        /* read the next input byte if there are no bits left */
        if (!bit_count) {
            if (read(input, &byte, sizeof(byte)) != sizeof(byte)) {
                perror("Unable to read the input file");
                return 1;
            }

            bit_count = MAX_BIT_SHIFT + 1;
        }

        /* consume one bit and traverse the tree */
        --bit_count; 
        cur_node = (byte & (1 << bit_count)) ? cur_node->right : cur_node->left;
    }

    return 0;
}

int main(int argc, char* argv[]) {
    int in_fd;
    int out_fd;
    FreqTable freq_table;
    int err;
    Node* root = NULL;

    /* Check the appropriate number of agrumens are passed */
    if (argc > 3) {
        fprintf(
            stderr,
            "Invalid arguments, should be: hdecode [(infile | -) [outfile]]\n"
        );

        return 1;
    }

    /* opens and creates a file descriptor for the input */
    in_fd = STDIN_FILENO;

    if ((argc > 1) && strcmp(argv[1], "-")) {
        in_fd = open(argv[1], O_RDONLY);

        if (in_fd == -1) {
            perror("Unable to open the input file");
            return 1;
        }
    }

    /* opens and creates a file descriptor for the output */
    out_fd = STDOUT_FILENO;

    if (argc == 3) {
        out_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);

        if (out_fd == -1) {
            perror("Unable to open the output file");

            if (in_fd != STDIN_FILENO) {
                close(in_fd);
            }

            return 1;
        }
    }

    /* read the frequency table from the header of the input file */
    err = read_freq_table(in_fd, &freq_table);

    if (!err) {
        /* create the huffman tree */
        root = create_linked_list(&freq_table);
        root = create_tree(root);

        /* read the input file and decode it */
        err = decode_file(in_fd, out_fd, &freq_table, root);
    }

    /* Close files and free memory */
    if (in_fd != STDIN_FILENO) {
        close(in_fd);
    }

    if (out_fd != STDOUT_FILENO) {
        close(out_fd);
    }

    free_tree(root);
    return err;
}
