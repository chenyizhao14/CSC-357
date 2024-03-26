#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <winsock2.h>

#include "huffman.h"

#define MAX_BIT_SHIFT 7

int write_freq_table(int output, FreqTable* freq_table) {
    unsigned value_count = count_number_of_values(freq_table);
    unsigned char header;
    unsigned char value;
    unsigned count;
    int i;

    /* if the input is empty, skip the header */
    if (!value_count) {
        return 0;
    }

    /* write the number of entries */
    header = value_count - 1;
    
    if (write(output, &header, sizeof(header)) != sizeof(header)) {
        perror("Unable to write to output file");
        return 1;
    }

    /* write the frequency table */
    for (
        i = 0; 
        i < sizeof(freq_table->table) / sizeof(*(freq_table->table));
        ++i
    ) {
        if (freq_table->table[i]) {
            value = i;

            if (write(output, &value, sizeof(value)) != sizeof(value)) {
                perror("Unable to write to output file");
                return 1;
            }

            count = htonl(freq_table->table[i]);

            if (write(output, &count, sizeof(count)) != sizeof(count)) {
                perror("Unable to write to output file");
                return 1;
            }
        }
    }

    return 0;
}

int encode_file(
    int input, 
    int output, 
    FreqTable* freq_table, 
    EncodingTable* encoding_table
) {
    unsigned bit_count = 0;
    unsigned char byte = 0;
    unsigned char c;
    char* p;

    /* read the input file and encode it */
    while (read(input, &c, sizeof(c)) == sizeof(c)) {
        if (!(freq_table->table[c])) {
            fprintf(stderr, "The input file has changed\n");
            return 1;
        }

        /* encode the current character */
        for (p = encoding_table->encoding[c]; (*p); ++p) {
            byte |= ((((*p) == '1') ? 1 : 0) << (MAX_BIT_SHIFT - bit_count));
            ++bit_count;

            /* write the byte to the output once there are 8 bits */
            if (bit_count > MAX_BIT_SHIFT) {
                if (write(output, &byte, sizeof(byte)) != sizeof(byte)) {
                    perror("Unable to write to the output file");
                    return 1;
                }

                bit_count = 0;
                byte = 0;
            }
        }
    }

    /* write the last byte, if there is one */
    if (bit_count) {
        if (write(output, &byte, sizeof(byte)) != sizeof(byte)) {
            perror("Unable to write to the output file");
            return 1;		
        }		
    }

    return 0;
}

int main(int argc, char* argv[]) {
    int in_fd;
    int out_fd;
    FreqTable freq_table;
    EncodingTable encoding_table;
    Node* root;
    unsigned char c;
    int err;

    /* Check the appropriate number of agrumens are passed */
    if ((argc < 2) || (argc > 3)) {
        fprintf(
            stderr,
            "Invalid arguments, should be: hencode infile [outfile]\n"
        );

        return 1;
    }

    /* opens and creates a file descriptor for the input */
    in_fd = open(argv[1], O_RDONLY);

    if (in_fd == -1) {
        perror("Unable to open the input file");
        return 1;
    }

    /* opens and creates a file descriptor for the output */
    out_fd = STDOUT_FILENO;

    if (argc == 3) {
        out_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);

        if (out_fd == -1) {
            perror("Unable to open the output file");
            close(in_fd);
            return 1;
        }
    }

    /* read the input file and build freqency table */
    init_freq_table(&freq_table);

    while (read(in_fd, &c, sizeof(c)) == sizeof(c)) {
        update_freq_table(&freq_table, c);
    }

    /* create the huffman tree and encodings */
    root = create_linked_list(&freq_table);
    root = create_tree(root);

    init_encoding_table(&encoding_table);
    create_encoding_table(root, &encoding_table);

    /* write the frequency table */
    err = write_freq_table(out_fd, &freq_table);
    
    if (!err) {
        if (lseek(in_fd, 0, SEEK_SET) < 0) {
            perror("Unable to read the input file");
            err = 1;
        } else {
            err = encode_file(in_fd, out_fd, &freq_table, &encoding_table);
        }
    }

    /* Close files and free memory */
    close(in_fd);

    if (out_fd != STDOUT_FILENO) {
        close(out_fd);
    }

    free_tree(root);
    return err;
}