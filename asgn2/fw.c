#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <unistd.h>

#include "hash.h"

#define DEFAULT_WORD_COUNT 10
#define DEFAULT_WORD_SIZE 20
#define HASH_TABLE_SIZE 2048

    /*logically speaking, for main to work we need:
    1. struct nodes that contain the word and frequency
    2. a data structure that contains all the nodes
    3. something to parse the command line for -n
    4. read the files to obtain each word
    5. insert the words into the data structure
    6. obtain the highest -n number of words that appear
    7. print out what is necessary*/

int compare_nodes(const void* a, const void* b) {
    Node* na = *((Node**)a);
    Node* nb = *((Node**)b);

    if (na->freq != nb->freq) {
        return nb->freq - na->freq;
    }

    return strcmp(nb->key, na->key);
}

char* read_long_word(FILE* file) {
    int c;
    unsigned len = 0, size = 0;
    char* s = NULL;
    char* new_s;

    for (c = fgetc(file); c != EOF; c = fgetc(file)) {
        if (!(isalpha(c))) {
            break;
        }
        if (len + 2 > size) {
            size += DEFAULT_WORD_SIZE;
            new_s = realloc(s, size);
            if (!new_s) {
                if (s) {
                    free(s);
                }
                return NULL;
            }
            s = new_s;
        }
        s[len++] = tolower(c);
    }

    if (s) {
        s[len] = 0;
    }

    return s;
}

int read_file(FILE* file, HashTable* table) {
    char* word;
    while (!feof(file)) {
        word = read_long_word(file);
        if (word) {
            if (!insert_key(table, word)) {
                return -1;
            }
        }
    }
    return 0;
}

int print_results(HashTable* table, unsigned word_count) {
    unsigned i;
    Node** bucket;
    Node* node;
    Node** p;
    Node** array = (Node**) malloc(sizeof(Node*) * table->num_nodes);
    if (!array) {
        return -1;
    }

    for (
        i = 0, bucket = table->buckets, p = array;
        i < table->capacity;
        ++i, ++bucket
    ) {
        for (node = (*bucket); node; node = node->next) {
            *(p++) = node;
        }
    }

    qsort(array, table->num_nodes, sizeof(Node*), compare_nodes);

    printf("The top %u words (out of %u) are:\n", word_count, table->num_nodes);

    if (table->num_nodes < word_count) {
        word_count = table->num_nodes;
    }

    for (i = 0, p = array; i < word_count; ++i, ++p) {
        printf("%9u %s\n", (*p)->freq, (*p)->key);
    }

    return 0;
}

/*optind: keeps track of the index of the next argument
atoi    : converts a string to an integer by taking in a single argument
optarg  : holds the argument for the most recent option that was parsed
atoi(optarg): converts string to an integer value, which is stored in 'n'*/

int main(int argc, char* argv[]) {
    unsigned n = DEFAULT_WORD_COUNT; /* default number of entries*/
    int opt;
    int i;
    int error;
    FILE* file;
    HashTable* table;

    /* parsing for -n command*/
    while ((opt = getopt(argc, argv, "n:")) != -1) {
        switch (opt) {
            case 'n':
                if (!sscanf(optarg, "%u", &n)) {
                    fprintf(stderr, "Usage: %s [-n num] [file...]\n", argv[0]);
                    return -1;
                }

                break;
                
            case '?':
                fprintf(stderr, "Usage: %s [-n num] [file...]\n", argv[0]);
                return -1;
        }
    }

    table = new_hash_table(HASH_TABLE_SIZE);
    if (!table) {
        return -1;
    }

    if (optind < argc) {
        for (i = optind; i < argc; ++i) {
            file = fopen(argv[i], "r");
            if (!file) {
                fprintf(stderr, "Error: Could not open file %s\n", argv[i]);
                continue;
            }

            if (read_file(file, table)) {
                fprintf(stderr, "Error: Could not read file %s\n", argv[i]);
            }

            fclose(file);
        }
    } else {
        if (read_file(stdin, table)) {
            fprintf(stderr, "Error: Could not read from stdin\n");
        }
    }
    
    error = print_results(table, n);
    free_table(table);

    return error;
}