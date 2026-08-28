#pragma once

#include <stdint.h>

#define FNV1_32A_INIT   ((uint32_t) 0x811c9dc5)
#define FNV1_64A_INIT   ((uint64_t) 0xcbf29ce484222325ULL)

uint32_t hash__fnv1a_32(uint32_t init, size_t buffer_len, void* buffer);
uint64_t hash__fnv1a_64(uint64_t init, size_t buffer_len, void* buffer);
