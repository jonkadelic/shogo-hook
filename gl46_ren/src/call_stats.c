#include "./call_stats.h"

#include <stddef.h>
#include <stdint.h>

#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_assert.h>

#include "logger.h"
#include "util/hash.h"

#define CALL_ALLOC_SIZE (16)

typedef struct call_entry {
    uint64_t name_hash;
    char const* name;
    size_t count;
} call_entry_t;

static struct {
    size_t entries_len, entries_capacity;
    call_entry_t* entries;
} CALL_STATS = { 0 };

static int compare_entries_by_hash(void const* a, void const* b);
static int compare_entries_by_count(void const* a, void const* b);

// name_hash must be first member for `compare_entries_by_hash` to work
static_assert(offsetof(call_entry_t, name_hash) == 0);

void call_stats__mark_called(char const* func_name) {
    auto self = &CALL_STATS;

    uint64_t name_hash = hash__fnv1a_64(FNV1_64A_INIT, strlen(func_name), func_name);
    call_entry_t* entry = SDL_bsearch(&name_hash, self->entries, self->entries_len, sizeof(call_entry_t), compare_entries_by_hash);
    if (entry == nullptr) {
        size_t entry_index = self->entries_len;
        if (self->entries_len + 1 > self->entries_capacity) {
            size_t new_entries_capacity = self->entries_capacity + CALL_ALLOC_SIZE;
            call_entry_t* new_entries = SDL_realloc(self->entries, sizeof(call_entry_t) * new_entries_capacity);
            if (new_entries == nullptr) {
                LOG_ERROR("Failed to alloc %zu call entries", new_entries_capacity);
                return;
            }
            
            self->entries_capacity = new_entries_capacity;
            self->entries = new_entries;
        }

        entry = &self->entries[entry_index];
        self->entries[entry_index].name_hash = name_hash;
        self->entries[entry_index].name = func_name;
        self->entries[entry_index].count = 0;
        self->entries_len++;

        // Re-sort entries
        SDL_qsort(self->entries, self->entries_len, sizeof(call_entry_t), compare_entries_by_hash);

        // Re-get entry
        entry = SDL_bsearch(&name_hash, self->entries, self->entries_len, sizeof(call_entry_t), compare_entries_by_hash);
        SDL_assert(entry != nullptr);
    }

    entry->count++;
}

void call_stats__log_all(void) {
    auto self = &CALL_STATS;

    call_entry_t* call_entries = self->entries;

    call_entry_t* clone = SDL_malloc(sizeof(call_entry_t) * self->entries_len);
    if (clone != nullptr) {
        SDL_memcpy(clone, self->entries, sizeof(call_entry_t) * self->entries_len);
        SDL_qsort(clone, self->entries_len, sizeof(call_entry_t), compare_entries_by_count);
        call_entries = clone;
    } else {
        LOG_WARNING("Unable to duplicate call entry list for sorting. Printing as unsorted...");
    }

    LOG_INFO("Call stats dump:");
    for (size_t i = 0; i < self->entries_len; i++) {
        LOG_INFO(" - %s: %zu", call_entries[i].name, call_entries[i].count);
    }

    SDL_free(clone); // safe
}

void call_stats__clear(void) {
    auto self = &CALL_STATS;
    self->entries_len = 0;
}

static int compare_entries_by_hash(void const* a, void const* b) {
    uint64_t const* hash_a = a;
    uint64_t const* hash_b = b;

    return (*hash_a > *hash_b) - (*hash_a < *hash_b);
}

static int compare_entries_by_count(void const* a, void const* b) {
    call_entry_t const* entry_a = a;
    call_entry_t const* entry_b = b;

    return (entry_a->count > entry_b->count) - (entry_a->count < entry_b->count);
}
