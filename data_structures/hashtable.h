#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stdlib.h>

#define TABLE_SIZE 100

/* A node in the hashtable */
typedef struct hash_node {
    char* key;
    void* value;
    struct hash_node* next;
} hash_node;

/* A hashtable structure */
typedef struct hashtable {
    hash_node* buckets[TABLE_SIZE];
} hashtable;

/* Hashes a key into an unsigned int */
unsigned int hash(const char* key);

/* Creates a hashtable and returns a pointer to the newly created table*/
hashtable* create_hashtable();

/* Inserts a new key-value pair into the hashtable, Note: it copies value by value into the table */
int insert(hashtable* ht, const char* key, const void* value, size_t value_size);

/* Searches for a key in the hashtable and returns the value, if the value is not found it returns NULL */
void* search(hashtable* ht, const char* key);

/* Replaces the value of a key in the hashtable with a new value */
void replace(hashtable* ht, const char* key, void* newValue, size_t newValueSize, int* allocationError);

/* Frees the hashtable */
void free_hashtable(hashtable* ht);

#endif /* HASHTABLE_H */