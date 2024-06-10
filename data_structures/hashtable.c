#ifndef HASHTABLE_C
#define HASHTABLE_C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../utils.h"

#define TABLE_SIZE 100

/* Hashes a key into an unsigned int, it uses djb2 hash algorithm */
unsigned int hash(const char* key) {
    unsigned long int value = 0;
    unsigned int i = 0;
    unsigned int key_len = strlen(key);

    for (; i < key_len; ++i) {
        value = value * 37 + key[i];
    }

    value = value % TABLE_SIZE;

    return value;
}

/* Creates a hashtable and returns a pointer to the newly created table*/
hashtable* create_hashtable() {
    int i;
    hashtable* ht;
    ht = malloc(sizeof(hashtable));
    if (ht == NULL) {
        return NULL;
    }
    for (i = 0; i < TABLE_SIZE; i++) {
        ht->buckets[i] = NULL;
    }
    return ht;
}

/* Inserts a new key-value pair into the hashtable, Note: it copies value by value into the table */
int insert(hashtable* ht, const char* key, const void* value, size_t value_size) {
    unsigned int bucket = hash(key);
    hash_node* newnode = malloc(sizeof(hash_node));
    if (newnode == NULL) {
        return 1;
    }
    newnode->key = duplicate_string(key);
    if (newnode->key == NULL) {
        free(newnode);
        return 1;
    }
    newnode->value = malloc(value_size);
    if (newnode->value == NULL) {
        free(newnode->key);
        free(newnode);
        return 1;
    }
    memcpy(newnode->value, value, value_size);
    newnode->next = ht->buckets[bucket];
    ht->buckets[bucket] = newnode;
    return 0;
}

/* Searches for a key in the hashtable and returns the value, if the value is not found it returns NULL */
void* search(hashtable* ht, const char* key) {
    unsigned int bucket = hash(key);
    hash_node* current = ht->buckets[bucket];
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            return current->value;
        }
        current = current->next;
    }
    /* Not found */
    return NULL; 
}

/* Replaces the value of a key in the hashtable with a new value */
void replace(hashtable* ht, const char* key, void* newValue, size_t newValueSize, int* allocationError) {
    unsigned int bucket = hash(key);
    hash_node* current = ht->buckets[bucket];
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            /* Free the old value and allocate memory for the new value */
            free(current->value);
            current->value = malloc(newValueSize);
            if (current->value == NULL) {
                return;
            }
            memcpy(current->value, newValue, newValueSize);
            return;
        }
        current = current->next;
    }
}

/* Frees the hashtable */
void free_hashtable(hashtable* ht) {
    int i;
    for (i = 0; i < TABLE_SIZE; i++) {
        hash_node* current = ht->buckets[i];
        while (current != NULL) {
            hash_node* temp = current;
            current = current->next;
            free(temp->key);
            free(temp->value);
            free(temp);
        }
    }
    free(ht);
}
#endif