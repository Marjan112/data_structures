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
    size_t buckets_capacity;
    size_t entries_count;
    size_t used_buckets;
    size_t resize_count;
    size_t max_entry_chain_len;
    size_t *buckets_chain_len;
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
    assert(ht->buckets_capacity != 0);
    return hash(key) % ht->buckets_capacity;
}

void ht_create(HashTable *ht, size_t capacity) {
    assert(ht);
    ht->buckets = calloc(capacity, sizeof(Bucket*));
    assert(ht->buckets);
    ht->buckets_capacity = capacity;
    ht->entries_count = 0;
    ht->used_buckets = 0;
    ht->resize_count = 0;
    ht->buckets_chain_len = calloc(capacity, sizeof(size_t));
    assert(ht->buckets_chain_len);
}

float ht_load_factor(const HashTable *ht) {
    assert(ht);
    return (float)ht->entries_count / (float)ht->buckets_capacity;
}

void ht_resize(HashTable *ht, size_t new_capacity) {
    assert(ht);
    assert(new_capacity != 0);
    assert(new_capacity > ht->buckets_capacity);

    Bucket **old_buckets = ht->buckets;
    size_t old_capacity = ht->buckets_capacity;

    ht->buckets = calloc(new_capacity, sizeof(Bucket*));
    assert(ht->buckets);
    ht->buckets_capacity = new_capacity;
    ht->entries_count = 0;
    ht->used_buckets = 0;
    ht->max_entry_chain_len = 0;

    free(ht->buckets_chain_len);
    ht->buckets_chain_len = calloc(new_capacity, sizeof(size_t));
    assert(ht->buckets_chain_len);

    for(size_t i = 0; i < old_capacity; ++i) {
        Bucket *current = old_buckets[i];
        while(current) {
            Bucket *next = current->next;

            size_t index = ht_get_index(ht, current->pair.key);

            if(ht->buckets[index] == NULL) ++ht->used_buckets;

            current->next = ht->buckets[index];
            ht->buckets[index] = current;
            ++ht->entries_count;

            if(++ht->buckets_chain_len[index] > ht->max_entry_chain_len) ht->max_entry_chain_len = ht->buckets_chain_len[index];

            current = next;
        }
    }

    free(old_buckets);

    ++ht->resize_count;
}

void ht_set(HashTable *ht, int key, char *value) {
    assert(ht);

    size_t index = ht_get_index(ht, key);
    Bucket *current = ht->buckets[index];
    while(current) {
        if(current->pair.key == key) {
            free(current->pair.value);
            current->pair.value = strdup(value);
            return;
        }
        current = current->next;
    }

    Bucket *new = malloc(sizeof(Bucket));
    assert(new);
    new->pair.key = key;
    new->pair.value = strdup(value);

    if(!ht->buckets[index]) ++ht->used_buckets;

    new->next = ht->buckets[index];
    ht->buckets[index] = new;

    ++ht->entries_count;

    if(++ht->buckets_chain_len[index] > ht->max_entry_chain_len) ht->max_entry_chain_len = ht->buckets_chain_len[index];
    if(ht_load_factor(ht) > 0.75f) ht_resize(ht, ht->buckets_capacity * 2);
}

char *ht_get(const HashTable *ht, int key) {
    size_t index = ht_get_index(ht, key);
    Bucket *current = ht->buckets[index];

    while(current) {
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

    while(current) {
        if(current->pair.key != key) {
            prev = current;
            current = current->next;
            continue;
        }

        if(prev == NULL) ht->buckets[index] = current->next;
        else prev->next = current->next;

        free(current->pair.value);
        free(current);

        --ht->entries_count;

        size_t old_len = ht->buckets_chain_len[index]--;
        if(ht->buckets_chain_len[index] == 0) --ht->used_buckets;
        if(old_len == ht->max_entry_chain_len) {
            size_t new_max = 0;
            for(size_t i = 0; i < ht->buckets_capacity; ++i) if(ht->buckets_chain_len[i] > new_max) new_max = ht->buckets_chain_len[i];
            ht->max_entry_chain_len = new_max;
        }

        return;
    }
}

void ht_destroy(HashTable *ht) {
    for(size_t i = 0; i < ht->buckets_capacity; ++i) {
        Bucket *current = ht->buckets[i];
        while(current) {
            Bucket *next = current->next;
            free(current->pair.value);
            free(current);
            current = next;
        }
    }
    free(ht->buckets);
    free(ht->buckets_chain_len);
    memset(ht, 0, sizeof(HashTable));
}

void ht_print(const HashTable *ht) {
    for(size_t i = 0; i < ht->buckets_capacity; ++i) {
        printf("[%zu] ", i);

        Bucket *current = ht->buckets[i];

        if(!current) {
            printf("(empty)\n");
            continue;
        }

        while(current) {
            printf("(%d, \"%s\")", current->pair.key, current->pair.value != NULL ? current->pair.value : "NULL");
            if(current->next != NULL) printf("->");
            current = current->next;
        }

        printf("\n");
    }
}

void ht_print_stats(const HashTable *ht) {
    printf("elements (entries): %zu\n", ht->entries_count);
    printf("buckets           : %zu\n", ht->buckets_capacity);
    printf("load factor       : %.2f\n", ht_load_factor(ht));
    printf("used buckets      : %zu (%.1f%%)\n", ht->used_buckets, 100.0f * ht->used_buckets / ht->buckets_capacity);
    printf("avg chain length  : %.2f\n", ht->used_buckets ? (float)ht->entries_count / ht->used_buckets : 0);
    printf("max chain length  : %zu\n", ht->max_entry_chain_len);
    printf("resize count      : %zu\n", ht->resize_count);
}

int main() {
    HashTable ht = {0};
    ht_create(&ht, 4);

    char str[128] = {0};

    for(size_t i = 0; i < 30000000; ++i) {
        sprintf(str, "str#%zu", i);
        ht_set(&ht, i, str);
    }

    ht_print_stats(&ht);

    ht_destroy(&ht);
    return 0;
}
