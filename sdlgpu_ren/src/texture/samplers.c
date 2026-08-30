#include "./samplers.h"

#include "util/hash.h"
#include "util/util.h"

#define SAMPLER_ALLOC_SIZE (4)

// config_hash must be first member for `compare_samplers_by_hash` to work
static_assert(offsetof(sampler_t, config_hash) == 0);

static int compare_samplers_by_hash(void const* a, void const* b);

static sampler_t* create_sampler(sampler_manager_t* self, sampler_config_t const* config, uint64_t config_hash);

bool sampler_manager__init(sampler_manager_t* self, SDL_GPUDevice* device) {
    ZERO_INIT_STRUCT(self);

    self->device = device;

    return true;
}

void sampler_manager__cleanup(sampler_manager_t* self) {
    for (size_t i = 0; i < self->samplers_len; i++) {
        SDL_ReleaseGPUSampler(self->device, self->samplers[i].sampler);
    }

    SDL_free(self->samplers);
    self->samplers = nullptr;
}

SDL_GPUSampler* sampler_manager__get_sampler(sampler_manager_t* self, sampler_config_t config) {
    uint64_t config_hash = hash__fnv1a_64(FNV1_64A_INIT, sizeof(config), &config);
    sampler_t* sampler = SDL_bsearch(
        &config_hash,
        self->samplers,
        self->samplers_len,
        sizeof(sampler_t),
        compare_samplers_by_hash
    );

    if (sampler == nullptr) {
        sampler = create_sampler(self, &config, config_hash);
        if (sampler == nullptr) {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "Could not create sampler");
            return nullptr;
        }
    }

    return sampler->sampler;
}

static int compare_samplers_by_hash(void const* a, void const* b) {
    uint64_t const* hash_a = a;
    uint64_t const* hash_b = b;

    return (*hash_a > *hash_b) - (*hash_a < *hash_b);
}

static sampler_t* create_sampler(sampler_manager_t* self, sampler_config_t const* config, uint64_t config_hash) {
    sampler_t* out = nullptr;

    // Ensure sampler buffer is large enough
    size_t sampler_index = self->samplers_len;
    if (self->samplers_len + 1 > self->samplers_capacity) {
        size_t new_samplers_capacity = self->samplers_capacity + SAMPLER_ALLOC_SIZE;
        sampler_t* new_samplers = SDL_realloc(self->samplers, sizeof(sampler_t) * new_samplers_capacity);
        if (new_samplers == nullptr) {
            SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "Failed to allocate %zu samplers", new_samplers_capacity);
            goto end;
        }

        self->samplers_capacity = new_samplers_capacity;
        self->samplers = new_samplers;
    }

    // Set up sampler create info
    SDL_GPUSamplerCreateInfo info = {
        .min_filter = config->min_filter,
        .mag_filter = config->mag_filter,
        .address_mode_u = config->wrap_s,
        .address_mode_v = config->wrap_t,
    };

    // Create sampler
    SDL_GPUSampler* sampler = SDL_CreateGPUSampler(self->device, &info);
    if (sampler == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_GPU, "Failed to create GPU sampler: %s", SDL_GetError());
    }

    // Init sampler object
    self->samplers[sampler_index].config_hash = config_hash;
    self->samplers[sampler_index].sampler = sampler;
    self->samplers_len++;

    // Sort samplers
    SDL_qsort(
        self->samplers,
        self->samplers_len,
        sizeof(sampler_t),
        compare_samplers_by_hash
    );
    
    // Get sampler back
    out = SDL_bsearch(
        &config_hash,
        self->samplers,
        self->samplers_len,
        sizeof(sampler_t),
        compare_samplers_by_hash
    );
    SDL_assert(out != nullptr);

end:
    return out;
}
