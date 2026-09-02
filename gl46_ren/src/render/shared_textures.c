#include "./shared_textures.h"

#include <stddef.h>

#include <SDL3/SDL.h>

#include "logger.h"
#include "render/render_api.h"
#include "util/hash.h"
#include "util/util.h"

#define TEXTURE_ALLOC_SIZE (4)

static int compare_textures_by_hash(void const* a, void const* b);

static shared_texture_t* create_texture(shared_texture_manager_t* self, TextureData_t const* texture_data, uint64_t path_hash);

bool shared_texture_manager__init(shared_texture_manager_t* self) {
    OBJECT_ZERO_INIT(self);

    return true;
}

void shared_texture_manager__cleanup(shared_texture_manager_t* self) {
    for (size_t i = 0; i < self->textures_len; i++) {
        texture__cleanup(&self->textures[i]->texture);
        SDL_free(self->textures[i]);
    }

    SDL_free(self->textures);
    self->textures = nullptr;
}

texture_t* shared_texture_manager__get_texture(shared_texture_manager_t* self, SharedTexture_t* lt_texture) {
    if (lt_texture->m_pFile == nullptr) {
        return nullptr;
    }

    uint64_t path_hash = hash__fnv1a_64(FNV1_64A_INIT, strlen(lt_texture->m_pFile->m_Filename), lt_texture->m_pFile->m_Filename);
    uint64_t const* path_hash_ptr = &path_hash;
    shared_texture_t** texture_ptr = SDL_bsearch(&path_hash_ptr, self->textures, self->textures_len, sizeof(shared_texture_t*), compare_textures_by_hash);
    shared_texture_t* texture = texture_ptr != nullptr ? *texture_ptr : nullptr;
    if (texture == nullptr) {
        auto texture_data = RENDER_STRUCT->GetTexture(lt_texture, 0);
        if (texture_data == nullptr) {
            LOG_WARNING("Failed to read texture data for shared texture at \"%s\"", lt_texture->m_pFile->m_Filename);
            return nullptr;
        }

        texture = create_texture(self, texture_data, path_hash);
        RENDER_STRUCT->FreeTexture(lt_texture);
        if (texture == nullptr) {
            LOG_ERROR("Failed to create new shared texture");
            return nullptr;
        }
    }

    SDL_assert(texture != nullptr);
    return &texture->texture;
}

static int compare_textures_by_hash(void const* a, void const* b) {
    uint64_t const* const* hash_a = a;
    shared_texture_t const* const* texture_b = b;

    uint64_t hash_b = (*texture_b)->path_hash;

    return (**hash_a > hash_b) - (**hash_a < hash_b);
}

static shared_texture_t* create_texture(shared_texture_manager_t* self, TextureData_t const* texture_data, uint64_t path_hash) {
    shared_texture_t* out = nullptr;
    uint32_t* tbuf = nullptr;

    // Ensure texture buffer is large enough
    size_t texture_index = self->textures_len;
    if (self->textures_len + 1 > self->textures_capacity) {
        size_t new_textures_capacity = self->textures_capacity + TEXTURE_ALLOC_SIZE;
        shared_texture_t** new_textures = SDL_realloc(self->textures, sizeof(shared_texture_t*) * new_textures_capacity);
        if (new_textures == nullptr) {
            LOG_ERROR("Failed to allocate %zu shared textures", new_textures_capacity);
            goto err;
        }

        self->textures_capacity = new_textures_capacity;
        self->textures = new_textures;
    }

    out = SDL_calloc(1, sizeof(shared_texture_t));
    if (out == nullptr) {
        LOG_ERROR("Failed to allocate shared texture");
        goto err;
    }
    self->textures[texture_index] = out;

    out->path_hash = path_hash;

    if (!texture__init(&out->texture, nullptr)) {
        LOG_ERROR("Failed to init shared texture");
        goto err;
    }

    tbuf = SDL_malloc(texture_data->m_Width * texture_data->m_Height * sizeof(uint32_t));
    if (tbuf == nullptr) {
        LOG_ERROR("Failed to alloc memory for shared texture data");
        goto err;
    }

    for (size_t i = 0; i < texture_data->m_Width * texture_data->m_Height; i++) {
        uint32_t rgba = texture_data->m_pPalette->m_Colors[texture_data->m_pDataBuffer[i]];
        uint32_t argb = ((rgba & 0xFFFFFF00) >> 8) | ((rgba & 0x000000FF) << 24);
        tbuf[i] = argb;
    }

    texture__upload(&out->texture, texture_data->m_Width, texture_data->m_Height, COLOR_FORMAT__RGBA32, tbuf);
    SDL_free(tbuf); tbuf = nullptr;

    self->textures_len++;

    // Sort textures
    SDL_qsort(
        self->textures,
        self->textures_len,
        sizeof(shared_texture_t*),
        compare_textures_by_hash
    );

    uint64_t const* path_hash_ptr = &path_hash;
    // Get texture back
    shared_texture_t** out_ptr = SDL_bsearch(
        &path_hash_ptr,
        self->textures,
        self->textures_len,
        sizeof(shared_texture_t*),
        compare_textures_by_hash
    );

    return out_ptr != nullptr ? *out_ptr : nullptr;

err:
    SDL_free(tbuf);
    SDL_free(out);
    return nullptr;
}