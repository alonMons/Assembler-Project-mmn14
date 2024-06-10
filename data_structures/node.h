#ifndef NODE_H
#define NODE_H

#include "../structs.h"

/* the node structure describes a node in a linked list */
typedef struct node {
    char* token;
    struct node* next;
} node;

/* the Symbol_Node structure describes a node in a linked list of symbols  */
typedef struct Symbol_Node {
    struct Symbol * symbol;
    struct Symbol_Node * next;
} Symbol_Node;

/* the External_Node structure describes a node in a linked list of externals  */
typedef struct External_Node {
    struct External * external;
    struct External_Node * next;
} External_Node;

/* Function to insert a new node into the linked list and return a pointer to the updated list */
node* insert_node(node* head, const char* token);

/* Function to check if a value is contained in the list */
int contains(node* head, const char* token);

/* Function to free the linked list */
void free_nodes(node* head);

/* the symbol_contains function searches for a symbol with a name that matches 
    label in the symbol table and returns it */
Symbol_Node * symbol_contains(Symbol_Node * head, const char* label);

/* the insert_symbol function inserts a new symbol into the symbol table and returns a pointer to it */
Symbol_Node * insert_symbol (Symbol_Node ** head, char * label);

/* the free_symbols function frees the memory that was assigned for the symbol table's nodes */
void free_symbols (Symbol_Node * head);

/* the insert_external function inserts a new external into the external table,
    or a new use of a present external, and returns a pointer to the head of the list */
External_Node * insert_external (External_Node * head, char * label, int IC);

/* the search_externals function searches for an external with a name that matches 
    label in the external table and returns it */
External_Node * search_externals (External_Node * head, char * label);

/* the free_externals function frees the memory that was assigned for the external table's nodes */
void free_externals (External_Node * head);

#endif