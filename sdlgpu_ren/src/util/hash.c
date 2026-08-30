#include "./hash.h"

#define FNV_32_PRIME ((uint32_t) 0x01000193)
#define FNV_64_PRIME ((uint64_t) 0x100000001b3ULL)

uint32_t hash__fnv1a_32(uint32_t init, size_t buffer_len, void const* buffer) {
    uint8_t const* bp = buffer;
    uint8_t const* be = bp + buffer_len;

    while (bp < be) {
        init ^= (uint32_t) *bp++;
        init *= FNV_32_PRIME;
    }

    return init;
}

uint64_t hash__fnv1a_64(uint64_t init, size_t buffer_len, void const* buffer) {
    uint8_t const* bp = buffer;
    uint8_t const* be = bp + buffer_len;

    while (bp < be) {
        init ^= (uint64_t) *bp++;
        init *= FNV_64_PRIME;
    }

    return init;
}
