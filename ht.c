#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

typedef struct {
    char *value;
    int key;
} KV;

typedef struct Bucket {
    KV pair;
    struct Bucket *next;
} Bucket;

typedef struct HashTable {
    Bucket **buckets;
    size_t capacity; // The number of buckets
    size_t count; // The number of occupied entries
} HashTable;

static inline size_t hash(int key) {
    size_t x = key;

    x ^= x >> 16;
    x *= 0x85ebca6b;
    x ^= x >> 13;
    x *= 0xc2b2ae35;
    x ^= x >> 16;

    return x;
}

size_t ht_get_index(const HashTable *ht, int key) {
    assert(ht->capacity != 0);
    return hash(key) & (ht->capacity - 1);
}

void ht_create(HashTable *ht, size_t capacity) {
    assert(ht != NULL && capacity != 0);
    ht->buckets = calloc(capacity, sizeof(Bucket*));
    ht->capacity = capacity;
    ht->count = 0;
}

float ht_load_factor(const HashTable *ht) {
    return (float)ht->count / (float)ht->capacity;
}

void ht_resize(HashTable *ht, size_t new_capacity) {
    Bucket **old_buckets = ht->buckets;
    size_t old_capacity = ht->capacity;

    ht->buckets = calloc(new_capacity, sizeof(Bucket*));
    ht->capacity = new_capacity;
    ht->count = 0;

    for(size_t i = 0; i < old_capacity; ++i) {
        Bucket *current = old_buckets[i];
        while(current) {
            Bucket *next = current->next;

            size_t index = current->pair.key % ht->capacity;
            current->next = ht->buckets[index];
            ht->buckets[index] = current;
            ++ht->count;

            current = next;
        }
    }

    free(old_buckets);
}

void ht_set(HashTable *ht, int key, char *value) {
    assert(ht != NULL);

    size_t index = ht_get_index(ht, key);
    Bucket *current = ht->buckets[index];

    while(current != NULL) {
        if(current->pair.key == key) {
            free(current->pair.value);
            current->pair.value = strdup(value);
            return;
        }
        current = current->next;
    }

    Bucket *new = malloc(sizeof(Bucket));
    new->pair.key = key;
    new->pair.value = strdup(value);

    new->next = ht->buckets[index];
    ht->buckets[index] = new;

    ++ht->count;

    if(ht_load_factor(ht) > 0.75) ht_resize(ht, ht->capacity * 2);
}

char *ht_get(const HashTable *ht, int key) {
    size_t index = ht_get_index(ht, key);
    Bucket *current = ht->buckets[index];

    while(current != NULL) {
        if(current->pair.key == key) {
            return current->pair.value;
        }
        current = current->next;
    }

    return NULL;
}

void ht_delete(HashTable *ht, int key) {
    size_t index = ht_get_index(ht, key);
    Bucket *current = ht->buckets[index];
    Bucket *prev = NULL;

    while(current != NULL) {
        if(current->pair.key != key) {
            prev = current;
            current = current->next;
            continue;
        }

        if(prev == NULL) ht->buckets[index] = current->next;
        else prev->next = current->next;

        free(current->pair.value);
        free(current);
        --ht->count;
        break;
    }
}

void ht_destroy(HashTable *ht) {
    for(size_t i = 0; i < ht->capacity; ++i) {
        Bucket *current = ht->buckets[i];
        while(current != NULL) {
            Bucket *next = current->next;
            free(current->pair.value);
            free(current);
            current = next;
        }
    }
    free(ht->buckets);
}

void ht_print(const HashTable *ht) {
    for(size_t i = 0; i < ht->capacity; ++i) {
        printf("[%zu] ", i);

        Bucket *current = ht->buckets[i];

        if(current == NULL) {
            printf("(empty)\n");
            continue;
        }

        while(current != NULL) {
            printf("(%d, \"%s\")", current->pair.key, current->pair.value != NULL ? current->pair.value : "NULL");

            if(current->next != NULL) printf("->");

            current = current->next;
        }

        printf("\n");
    }
}

void ht_print_stats(const HashTable *ht) {
    size_t max = 0;
    size_t used = 0;
    size_t total = 0;

    for(size_t i = 0; i < ht->capacity; ++i) {
        size_t len = 0;
        Bucket *current = ht->buckets[i];

        while(current != NULL) {
            ++len;
            current = current->next;
        }

        if(len) ++used;
        if(len > max) max = len;
        total += len;
    }

    printf("elements (entries): %zu\n", ht->count);
    printf("buckets           : %zu\n", ht->capacity);
    printf("load factor       : %.2f\n", ht_load_factor(ht));
    printf("used buckets      : %zu (%.1f%%)\n", used, 100.0f * used / ht->capacity);
    printf("avg chain length  : %.2f\n", used ? (float)total / used : 0);
    printf("max chain length  : %zu\n", max);
}

int main() {
    HashTable ht = {0};
    ht_create(&ht, 4);

    ht_set(&ht, 14, "miki");
    ht_set(&ht, 53, "misa");
    ht_set(&ht, 15, "masa");
    ht_set(&ht, 27, "sasa");

    ht_print(&ht);

    ht_print_stats(&ht);

    ht_destroy(&ht);
    return 0;
}
