#ifndef HASHMAP_H_
#define HASHMAP_H_

#include <lib/types.h>
#include <lib/string.h>
#include <mm/heap.h>


uint32_t hashmap_hash(const void* data, size_t length);

#define HASHMAP_KEY_DATA_MAX 256

#define HASHMAP_INIT(CAP) { .cap = (CAP), .buckets = NULL }

#define HASHMAP_TYPE(TYPE) \
    struct { \
        size_t cap; \
        struct { \
            size_t cap; \
            size_t filled; \
            struct { \
                uint8_t key_data[HASHMAP_KEY_DATA_MAX]; \
                size_t key_length; \
                TYPE item; \
            } *items; \
        } *buckets; \
    }

#define HASHMAP_GET(HASHMAP, RET, KEY_DATA, KEY_LENGTH) ({ \
    bool HASHMAP_GET_ok = false; \
    \
    __auto_type HASHMAP_GET_key_data = KEY_DATA; \
    __auto_type HASHMAP_GET_key_length = KEY_LENGTH; \
    \
    __auto_type HASHMAP_GET_hashmap = HASHMAP; \
    \
    size_t HASHMAP_GET_hash = hashmap_hash(HASHMAP_GET_key_data, HASHMAP_GET_key_length); \
    size_t HASHMAP_GET_index = HASHMAP_GET_hash % HASHMAP_GET_hashmap->cap; \
    \
    __auto_type HASHMAP_GET_bucket = &HASHMAP_GET_hashmap->buckets[HASHMAP_GET_index]; \
    \
    for (size_t HASHMAP_GET_i = 0; HASHMAP_GET_i < HASHMAP_GET_bucket->filled; HASHMAP_GET_i++) { \
        if (HASHMAP_GET_key_length != HASHMAP_GET_bucket->items[HASHMAP_GET_i].key_length) { \
            continue; \
        } \
        if (kmemcmp(HASHMAP_GET_key_data, \
                    HASHMAP_GET_bucket->items[HASHMAP_GET_i].key_data, \
                    HASHMAP_GET_key_length) == 0) { \
            RET = HASHMAP_GET_bucket->items[HASHMAP_GET_i].item; \
            HASHMAP_GET_ok = true; \
            break; \
        } \
    } \
    \
    HASHMAP_GET_ok; \
})

#define HASHMAP_SGET(HASHMAP, RET, STRING) ({ \
    const char *HASHMAP_SGET_string = (STRING); \
    HASHMAP_GET(HASHMAP, RET, HASHMAP_SGET_string, kstrlen(HASHMAP_SGET_string)); \
})

#define HASHMAP_INSERT(HASHMAP, KEY_DATA, KEY_LENGTH, ITEM) do { \
    __auto_type HASHMAP_INSERT_key_data = KEY_DATA; \
    __auto_type HASHMAP_INSERT_key_length = KEY_LENGTH; \
    \
    __auto_type HASHMAP_INSERT_hashmap = HASHMAP; \
    if (HASHMAP_INSERT_hashmap->buckets == NULL) { \
        HASHMAP_INSERT_hashmap->buckets = \
            kmalloc(HASHMAP_INSERT_hashmap->cap * sizeof(*HASHMAP_INSERT_hashmap->buckets)); \
    } \
    \
    size_t HASHMAP_INSERT_hash = hashmap_hash(HASHMAP_INSERT_key_data, HASHMAP_INSERT_key_length); \
    size_t HASHMAP_INSERT_index = HASHMAP_INSERT_hash % HASHMAP_INSERT_hashmap->cap; \
    \
    __auto_type HASHMAP_INSERT_bucket = &HASHMAP_INSERT_hashmap->buckets[HASHMAP_INSERT_index]; \
    \
    if (HASHMAP_INSERT_bucket->cap == 0) { \
        HASHMAP_INSERT_bucket->cap = 16; \
        HASHMAP_INSERT_bucket->items = \
            kmalloc(HASHMAP_INSERT_bucket->cap * sizeof(*HASHMAP_INSERT_bucket->items)); \
    } \
    \
    if (HASHMAP_INSERT_bucket->filled == HASHMAP_INSERT_bucket->cap) { \
        HASHMAP_INSERT_bucket->cap *= 2; \
        HASHMAP_INSERT_bucket->items = \
            krealloc(HASHMAP_INSERT_bucket->items, \
                    HASHMAP_INSERT_bucket->cap * sizeof(*HASHMAP_INSERT_bucket->items)); \
    } \
    \
    __auto_type HASHMAP_INSERT_item = &HASHMAP_INSERT_bucket->items[HASHMAP_INSERT_bucket->filled]; \
    \
    kmemcpy(HASHMAP_INSERT_item->key_data, HASHMAP_INSERT_key_data, HASHMAP_INSERT_key_length); \
    HASHMAP_INSERT_item->key_length = HASHMAP_INSERT_key_length; \
    HASHMAP_INSERT_item->item = ITEM; \
    \
    HASHMAP_INSERT_bucket->filled++; \
} while (0)

#define HASHMAP_SINSERT(HASHMAP, STRING, ITEM) do { \
    const char *HASHMAP_SINSERT_string = (STRING); \
    HASHMAP_INSERT(HASHMAP, HASHMAP_SINSERT_string, kstrlen(HASHMAP_SINSERT_string), ITEM); \
} while (0)

#endif
