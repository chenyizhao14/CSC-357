#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "huffman.h"
 
void init_freq_table(FreqTable* table) {
    /* set all values of table to 0 */
    memset(table, 0, sizeof(*table));
}

void update_freq_table(FreqTable* table, unsigned char value) {
    /* update the frequency for the given byte */
    ++(table->table[value]);
}

unsigned count_number_of_values(FreqTable* table) {
    /* count the number of non-zero frequencies */
    int i;
    unsigned count = 0;
    
    for (i = 0; i < sizeof(table->table) / sizeof(*(table->table)); ++i) {
        if (table->table[i]) {
            ++count;
        }
    }

    return count;
}

int compare_nodes(const void* a, const void* b) {
    Node* na = *((Node**)a);
    Node* nb = *((Node**)b);

    /* compare frequencies first */
    if (na->freq != nb->freq) {
        return na->freq - nb->freq;
    }

    /* then compare the values */
    return na->value - nb->value;    
}

Node* create_linked_list(FreqTable* table) {
    /* convert frequency table to an array of nodes */
    unsigned node_count = count_number_of_values(table);
    Node** nodes;
    int i; 
    Node** next_slot;
    Node* first_node;

    if (!node_count) {
        return NULL;
    }

    nodes = (Node**) malloc(node_count * sizeof(Node*));
    next_slot = nodes;

    for (i = 0; i < sizeof(table->table) / sizeof(*(table->table)); ++i) {
        if (table->table[i]) {
            (*next_slot) = (Node*) calloc(1, sizeof(Node));
            (*next_slot)->freq = table->table[i]; 
            (*next_slot)->value = i; 
            ++next_slot;
        }
    }

    /* sort array by their frequencies and values */
    qsort(nodes, node_count, sizeof(Node*), compare_nodes);

    /* convert array to linked list */
    first_node = NULL;

    for (i = node_count - 1; i >= 0; --i) {
        nodes[i]->next = first_node;
        first_node = nodes[i];
    }

    free(nodes);
    return first_node;
}

Node* insert_into_linked_list(Node* head, Node* new_node) {
    Node** cur;

    for (cur = (&head); (*cur); cur = &((*cur)->next)) {
        if ((*cur)->freq >= new_node->freq) {
            /* insert new node before the current node */
            new_node->next = (*cur);
            (*cur) = new_node;

            return head;
        }
    } 

    /* insert new node at the end of the list */
    (*cur) = new_node;
    new_node->next = NULL;

    return head;
}

Node* create_tree(Node* head) {
    /* should return the root node pointer */
    Node* new_list_head;
    Node* new_node;

    if (!head) {
        return NULL;
    }

    while (head->next) {
        new_node = (Node*) calloc(1, sizeof(Node));
        new_node->left = head;
        new_node->right = head->next;
        new_node->freq = new_node->left->freq + new_node->right->freq;

        new_list_head = new_node->right->next;
        new_node->left->next = NULL;
        new_node->right->next = NULL;

        head = insert_into_linked_list(new_list_head, new_node);
    }

    return head;
}

void free_tree(Node* root) {
    if (!root) {
        return;
    }

    free_tree(root->left);
    free_tree(root->right);
    free(root);
}

void init_encoding_table(EncodingTable* table) {
    /* set all values of table to 0 */
    memset(table, 0, sizeof(*table));
}

void traverse_tree(Node* node, EncodingTable* encoding_table, char* prefix) {
    char new_prefix[MAX_HUFFMAN_BITS + 1];

    /* leaf node */
    if (!(node->left)) {
        strcpy(encoding_table->encoding[node->value], prefix);
        return;
    } 

    /* traverse left */
    strcpy(new_prefix, prefix);
    strcat(new_prefix, "0");
    traverse_tree(node->left, encoding_table, new_prefix);

    /*traverse right */
    strcpy(new_prefix, prefix);
    strcat(new_prefix, "1");
    traverse_tree(node->right, encoding_table, new_prefix);
}

void create_encoding_table(Node* root, EncodingTable* encoding_table) {
    if (root) {
        traverse_tree(root, encoding_table, "");
    }
}