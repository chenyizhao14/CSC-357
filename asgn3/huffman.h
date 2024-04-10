#ifndef __HUFFMAN_H__
#define __HUFFMAN_H__

#define NUMBER_OF_BYTE_VALUES 256
#define MAX_HUFFMAN_BITS NUMBER_OF_BYTE_VALUES

typedef struct t_node {
    unsigned char value;
    unsigned freq;
    struct t_node* next;
    struct t_node* left;
    struct t_node* right;
} Node;

typedef struct t_freq_table {
    unsigned table[NUMBER_OF_BYTE_VALUES];
} FreqTable;

typedef struct t_encoding_table {
    char encoding[NUMBER_OF_BYTE_VALUES][MAX_HUFFMAN_BITS + 1];
} EncodingTable;

void init_freq_table(FreqTable* table);
void update_freq_table(FreqTable* table, unsigned char value);
unsigned count_number_of_values(FreqTable* table);
Node* create_linked_list(FreqTable* table);
Node* insert_into_linked_list(Node* head, Node* new_node);
Node* create_tree(Node* head);
void free_tree(Node* root);
void init_encoding_table(EncodingTable* table);
void traverse_tree(Node* node, EncodingTable* encoding_table, char* prefix);
void create_encoding_table(Node* root, EncodingTable* encoding_table);

#endif