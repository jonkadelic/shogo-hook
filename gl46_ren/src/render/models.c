#include "./models.h"

#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_assert.h>

#include "logger.h"
#include "util/hash.h"
#include "util/util.h"

#define MODEL_ALLOC_SIZE (4)

// Ensures `path_hash` is the first member of `model_manager_entry_t`
static_assert(offsetof(model_manager_entry_t, path_hash) == 0x00);

static int compare_entries_by_path_hash(void const* a, void const* b);

static model_manager_entry_t* create_model(model_manager_t* self, ModelData_t const* model_data, uint64_t path_hash);

bool model_manager__init(model_manager_t* self) {
    OBJECT_ZERO_INIT(self);

    return true;
}

void model_manager__cleanup(model_manager_t* self) {
    for (size_t i = 0; i < self->models_len; i++) {
        model_renderer__cleanup(&self->models[i]->renderer);
    }
    SDL_free(self->models);
    self->models = nullptr;
    self->models_len = 0;
    self->models_capacity = 0;
}

model_renderer_t* model_manager__get_renderer(model_manager_t* self, ModelData_t const* model_data) {
    char* path_upper = SDL_strdup(model_data->m_pFilename);
    if (path_upper == nullptr) {
        LOG_ERROR("Failed to alloc uppercase string");
    }
    SDL_strupr(path_upper);

    uint64_t path_hash = hash__fnv1a_64(FNV1_64A_INIT, SDL_strlen(path_upper), path_upper);
    uint64_t const* path_hash_ptr = &path_hash;
    model_manager_entry_t** model_ptr = SDL_bsearch(&path_hash_ptr, self->models, self->models_len, sizeof(model_manager_entry_t*), compare_entries_by_path_hash);
    model_manager_entry_t* model = model_ptr != nullptr ? *model_ptr : nullptr;
    if (model == nullptr) {
        model = create_model(self, model_data, path_hash);
        if (model == nullptr) {
            LOG_ERROR("Failed to create new model renderer");
            goto err;
        }

        LOG_DEBUG("Loaded model with name \"%s\"", path_upper);
    }

    SDL_free(path_upper);

    SDL_assert(model != nullptr);
    return &model->renderer;

err:
    SDL_free(path_upper);
    return nullptr;
}

model_renderer_t* model_manager__get_renderer_by_filename(model_manager_t* self, char const* filename) {
    char* path_upper = SDL_strdup(filename);
    if (path_upper == nullptr) {
        LOG_ERROR("Failed to alloc uppercase string");
        goto err;
    }
    SDL_strupr(path_upper);

    uint64_t path_hash = hash__fnv1a_64(FNV1_64A_INIT, SDL_strlen(path_upper), path_upper);
    uint64_t const* path_hash_ptr = &path_hash;
    model_manager_entry_t** model_ptr = SDL_bsearch(&path_hash_ptr, self->models, self->models_len, sizeof(model_manager_entry_t*), compare_entries_by_path_hash);
    model_manager_entry_t* model = model_ptr != nullptr ? *model_ptr : nullptr;
    if (model == nullptr) {
        LOG_WARNING("Tried to get non-loaded model at \"%s\"", path_upper);
        goto err;
    }

    SDL_free(path_upper);
    return &model->renderer;

err:
    SDL_free(path_upper);
    return nullptr;
}

static int compare_entries_by_path_hash(void const* a, void const* b) {
    uint64_t const* const* hash_a = a;
    uint64_t const* const* hash_b = b;

    return (**hash_a > **hash_b) - (**hash_a < **hash_b);
}

static model_manager_entry_t* create_model(model_manager_t* self, ModelData_t const* model_data, uint64_t path_hash) {
    model_manager_entry_t* out = nullptr;

    // Ensure model buffer is large enough
    size_t model_index = self->models_len;
    if (self->models_len + 1 > self->models_capacity) {
        size_t new_models_capacity = self->models_capacity + MODEL_ALLOC_SIZE;
        model_manager_entry_t** new_models = SDL_realloc(self->models, sizeof(model_manager_entry_t*) * new_models_capacity);
        if (new_models == nullptr) {
            LOG_ERROR("Failed to allocate %zu model entries", new_models_capacity);
            goto err;
        }

        self->models_capacity = new_models_capacity;
        self->models = new_models;
    }

    out = SDL_calloc(1, sizeof(model_manager_entry_t));
    if (out == nullptr) {
        LOG_ERROR("Failed to alloc model entry");
        goto err;
    }
    self->models[model_index] = out;
    
    out->path_hash = path_hash;

    if (!model_renderer__init(&out->renderer, model_data)) {
        LOG_ERROR("Failed to init model renderer");
        goto err;
    }

    self->models_len++;

    // Sort models
    SDL_qsort(
        self->models,
        self->models_len,
        sizeof(model_manager_entry_t*),
        compare_entries_by_path_hash
    );

    uint64_t const* path_hash_ptr = &path_hash;
    // Get model back
    model_manager_entry_t** out_ptr = SDL_bsearch(
        &path_hash_ptr,
        self->models,
        self->models_len,
        sizeof(model_manager_entry_t*),
        compare_entries_by_path_hash
    );

    return out_ptr != nullptr ? *out_ptr : nullptr;

err:
    SDL_free(out);
    return nullptr;
}
