#include <stdlib.h>
#include <string.h>

#include "hash.h"

unsigned hash(char* string) {
    char* p;
    unsigned hash = 0;
    for (p = string; *p; ++p) {
        hash = 31 * hash + (*p);
    }
    return hash;
}

HashTable* new_hash_table(unsigned capacity) {
    /*hashtable is a table containing pointers to nodes
    we have a pointer pointing to the hashtable pointing pointers to node*/

    HashTable* htable = (HashTable*) calloc(1, sizeof(HashTable));
    if (!htable) {
        return NULL;
    }

    htable->capacity = capacity;
    htable->buckets = (Node**) calloc(capacity, sizeof(Node*));

    if (!(htable->buckets)) {
        free(htable);
        return NULL;
    }

    /*calloc 0s everything out for you :D*/
    return htable;
}

void free_table(HashTable* table) {
    unsigned i;
    Node* node;
    Node* p;
    Node** bucket;
    for (i = 0, bucket = table->buckets; i < table->capacity; ++i, ++bucket) {
        for (node = (*bucket); node;) {
            p = node;
            node = node->next;
            free(p->key);
            free(p);
        }
    }
    free(table->buckets);
    free(table);
}

unsigned insert_key(HashTable* table, char* key) {
    unsigned index = hash(key) % table->capacity;
    Node** p;

    for (p = table->buckets + index; *p; p = &((*p)->next)) {
        if (!strcmp((*p)->key, key)) {
            ++((*p)->freq);
            free(key);
            return (*p)->freq;
        }
    }

    (*p) = (Node*) malloc(sizeof(Node));
    if (!(*p)) {
        return 0;
    }

    (*p)->key = key;
    (*p)->freq = 1;
    (*p)->next = NULL;

    ++(table->num_nodes);
    return (*p)->freq;
}
