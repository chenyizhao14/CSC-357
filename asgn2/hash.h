#ifndef __HASH_H__
#define __HASH_H__

typedef struct t_node {
    char* key;
    unsigned freq;
    struct t_node* next;
} Node;

typedef struct hash_table {
    unsigned capacity;
    unsigned num_nodes;
    Node** buckets;
 } HashTable;

unsigned hash(char* string);
HashTable* new_hash_table(unsigned capacity);
unsigned insert_key(HashTable* table, char* key);
void free_table(HashTable* table);

#endif