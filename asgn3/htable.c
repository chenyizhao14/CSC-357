#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "huffman.h"

void print_htable(FreqTable* freq_table, EncodingTable* encoding_table) {
    int i;

    for (i = 0; i < NUMBER_OF_BYTE_VALUES; ++i) {
        if (freq_table->table[i]) {
            printf("0x%02x: %s\n", i, encoding_table->encoding[i]);
        }
    }
}

int main(int argc, char* argv[]) {
    /* Parts necessary to complete lab
    1. read files
    2. create array
    3. create tree
    4. create table*/

    FreqTable freq_table;
    EncodingTable encoding_table;
    FILE* f;
    int c;
    Node* root;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input file>\n", argv[0]);
        return 1;
    }

    f = fopen(argv[1], "r");

    if (!f) {
        fprintf(stderr, "Error opening file %s\n", argv[1]);
        return 1;
    }

    init_freq_table(&freq_table);

    while ((c = fgetc(f)) != EOF) {
        update_freq_table(&freq_table, c);
    }

    fclose(f);

    root = create_linked_list(&freq_table);
    root = create_tree(root);

    init_encoding_table(&encoding_table);
    create_encoding_table(root, &encoding_table);
    print_htable(&freq_table, &encoding_table);
    free_tree(root);

    return 0;
}
