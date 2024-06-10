#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "node.h"
#include "../utils.h"
#include "../structs.h"

/* Function to insert a new node into the linked list and return a pointer to the updated list */
node* insert_node(node* head, const char* token) {
    node *newNode, *current;
    newNode = malloc(sizeof(node));
    if (newNode == NULL) {
        return NULL;
    }
    newNode->token = duplicate_string(token);
    if (newNode->token == NULL) {
        free(newNode);
        return NULL;
    }
    newNode->next = NULL;

    if (head == NULL) {
        return newNode;
    } else {
        current = head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
        return head;
    }
}

/* Function to check if a value is contained in the list */
int contains(node* head, const char* token) {
    node *current;
    current = head;
    while (current != NULL) {
        if (strcmp(current->token, token) == 0) {
            return 1;
        }
        current = current->next;
    }
    return 0;
}

/* Function to free the linked list */
void free_nodes(node* head) {
    node *current, *next;
    current = head;
    while (current != NULL) {
        next = current->next;
        free(current->token); /* Free the token string */
        free(current); /* Free the node itself */
        current = next;
    }
}

/* the symbol_contains function searches for a symbol with a name that matches 
    label in the symbol table and returns it */
Symbol_Node * symbol_contains(Symbol_Node * head, const char* label) {
        Symbol_Node * current;
        current = head;
        while (current != NULL) {
            if (current->symbol != NULL && strcmp(current->symbol->name, label) == 0) {
                return current;
            }
            current = current->next;
        }
    return NULL;
}

/* the insert_symbol function inserts a new symbol into the symbol table and returns a pointer to it */
Symbol_Node * insert_symbol (Symbol_Node ** head, char * label) {
    if (*head == NULL) {
        /* if the list is empty */
        Symbol_Node * symbol;
        symbol = malloc(sizeof(Symbol_Node));
        if (symbol == NULL) {
            return NULL;
        }
        symbol->symbol = malloc(sizeof(Symbol));
        if (symbol->symbol == NULL) {
            free(symbol);
            return NULL;
        }
        strcpy(symbol->symbol->name, label);
        symbol->next = NULL;
        *head = symbol;
        return symbol;
    } else {
        Symbol_Node * current;
        Symbol_Node * symbol;
        current = *head;
        while (current->next != NULL) {
            /* go to the end of the list */
            current = current->next;
        }
        symbol = malloc(sizeof(Symbol_Node));
        if (symbol == NULL) {
            return NULL;
        }
        symbol->symbol = malloc(sizeof(Symbol));
        if (symbol->symbol == NULL) {
            free(symbol);
            return NULL;
        }
        strcpy(symbol->symbol->name, label);
        current->next = symbol;
        symbol->next = NULL;
        return symbol;
    } 
}

/* the free_symbols function frees the memory that was assigned for the symbol table's nodes */
void free_symbols (Symbol_Node * head) {
    Symbol_Node * current1;
    Symbol_Node * current2;
    current1 = head;
    current2 = head;
    while (current1 != NULL) {
        free(current1->symbol);
        current1 = current1->next;
        free(current2);
        current2 = current1;
    }
}

/* the insert_external function inserts a new external into the external table,
    or a new use of a present external, and returns a pointer to the head of the list */
External_Node * insert_external (External_Node * head, char * label, int IC) {
    External_Node * found;
    found = search_externals(head, label);
    if (found) {
        /* if the external is already in the list */
        found->external->addresses[found->external->numOfUse] = IC; /* add the new use */
        found->external->numOfUse++;
        return head;
    } else {
        /* if the external is not in the list */
        External_Node * current;
        External_Node * newNode;
        current = head;
        while (current != NULL && current->next != NULL) {
            /* go to the end of the list */
            current = current->next;
        }
        newNode = malloc(sizeof(External_Node));
        newNode->external = malloc(sizeof(External));
        newNode->external->name = duplicate_string(label);
        newNode->external->addresses[0] = IC;
        newNode->external->numOfUse = 1;
        newNode->next = NULL;
        if (current) {
            current->next = newNode;
        }
        if (head == NULL) {
            return newNode;
        } else {
            return head;
        }
    }  
}

/* the search_externals function searches for an external with a name that matches 
    label in the external table and returns it */
External_Node * search_externals (External_Node * head, char * label) {
    External_Node * current;
    current = head;
    while (current != NULL) {
        if (strcmp(current->external->name, label) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/* the free_externals function frees the memory that was assigned for the external table's nodes */
void free_externals (External_Node * head) {
    External_Node * current1;
    External_Node * current2;
    current1 = head;
    current2 = head;
    while (current1 != NULL) {
        free(current1->external->name);
        free(current1->external);
        current1 = current1->next;
        free(current2);
        current2 = current1;
    }
}
