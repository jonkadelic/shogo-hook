#include "./rsurfaces.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL3/SDL.h>

#include "logger.h"
#include "util/util.h"

#define RSURFACE_ALLOC_NUM (4)

// Ensures an rsurface_id_t can be passed back from r_CreateSurface
static_assert(sizeof(void*) >= sizeof(rsurface_id_t));

// Ensures `id` is the first member of `rsurface_id_t`
static_assert(offsetof(rsurface_t, id) == 0x00);

static int compare_surfaces_by_id(void const* a, void const* b);

bool rsurface_manager__init(rsurface_manager_t* self) {
    OBJECT_ZERO_INIT(self);

    self->next_id = 1;

    return true;
}

void rsurface_manager__cleanup(rsurface_manager_t* self) {
    for (size_t i = 0; i < self->rsurfaces_capacity; i++) {
        texture__cleanup(&self->rsurfaces[i].texture);
    }

    SDL_free(self->rsurfaces);
    self->rsurfaces = nullptr;
    self->rsurfaces_capacity = 0;
}

rsurface_id_t rsurface_manager__create_rsurface(rsurface_manager_t* self, int32_t width, int32_t height) {
    if (width <= 0 || height <= 0) {
        return RSURFACE_ID_INVALID;
    }

    rsurface_t* rsurface = nullptr;
    for (size_t i = 0; i < self->rsurfaces_capacity; i++) {
        if (self->rsurfaces[i].id == RSURFACE_ID_INVALID) {
            rsurface = &self->rsurfaces[i];
            break;
        }
    }

    if (rsurface == nullptr) {
        size_t new_rsurfaces_len = self->rsurfaces_capacity + RSURFACE_ALLOC_NUM;
        rsurface_t* new_rsurfaces = SDL_realloc(self->rsurfaces, sizeof(rsurface_t) * new_rsurfaces_len);
        if (new_rsurfaces == nullptr) {
            LOG_ERROR("Failed to reallocate %zu rsurfaces", new_rsurfaces_len);
            goto err;
        }

        SDL_memset(new_rsurfaces + self->rsurfaces_capacity, 0, RSURFACE_ALLOC_NUM * sizeof(rsurface_t));

        rsurface = &new_rsurfaces[self->rsurfaces_capacity];

        self->rsurfaces_capacity = new_rsurfaces_len;
        self->rsurfaces = new_rsurfaces;
    }

    rsurface_id_t id = self->next_id++;
    rsurface->id = id;
    rsurface->width = width;
    rsurface->height = height;

    if (!texture__init(&rsurface->texture, nullptr)) {
        LOG_ERROR("Failed to create rsurface texture");
        goto err;
    }

    if (!pixel_buffer__init(&rsurface->buffer, width, height, COLOR_FORMAT__RGBA32)) {
        LOG_ERROR("Failed to create rsurface buffer");
        goto err;
    }

    // Re-sort buffers
    SDL_qsort(self->rsurfaces, self->rsurfaces_capacity, sizeof(rsurface_t), compare_surfaces_by_id);

    return id;

err:
    if (rsurface != nullptr) {
        pixel_buffer__cleanup(&rsurface->buffer);
        texture__cleanup(&rsurface->texture);
    }
    return RSURFACE_ID_INVALID;
}

void rsurface_manager__delete_rsurface(rsurface_manager_t* self, rsurface_id_t id) {
    if (id == RSURFACE_ID_INVALID) return;

    rsurface_t* rsurface = SDL_bsearch(
        &id,
        self->rsurfaces, 
        self->rsurfaces_capacity, sizeof(rsurface_t),
        compare_surfaces_by_id
    );
    if (rsurface == nullptr) return;

    pixel_buffer__cleanup(&rsurface->buffer);
    texture__cleanup(&rsurface->texture);
    SDL_memset(rsurface, 0, sizeof(rsurface_t));

    // Re-sort buffers
    SDL_qsort(self->rsurfaces, self->rsurfaces_capacity, sizeof(rsurface_t), compare_surfaces_by_id);
}

rsurface_t* rsurface_manager__get_rsurface(rsurface_manager_t* self, rsurface_id_t id) {
    if (id == RSURFACE_ID_INVALID) return nullptr;

    return SDL_bsearch(
        &id,
        self->rsurfaces, 
        self->rsurfaces_capacity, sizeof(rsurface_t),
        compare_surfaces_by_id
    );
}

void rsurface_manager__update_texture(rsurface_manager_t* self, rsurface_id_t id, bool transparency, uint16_t transparent_color) {
    if (id == RSURFACE_ID_INVALID) return;

    rsurface_t* rsurface = SDL_bsearch(
        &id,
        self->rsurfaces,
        self->rsurfaces_capacity,
        sizeof(rsurface_t),
        compare_surfaces_by_id
    );
    if (rsurface == nullptr) {
        return;
    }

    if (rsurface->width <= 0 || rsurface->height <= 0) {
        return;
    }

    if (!(rsurface->locked || rsurface->texture.gl_texture == 0)) {
        return;
    }

    uint32_t* buffer = SDL_malloc(rsurface->width * rsurface->height * sizeof(uint32_t));
    if (buffer == nullptr) {
        LOG_ERROR("Failed to allocate copy buffer for rsurface");
        return;
    }

    uint16_t const* extern_data = (uint16_t const*) rsurface->buffer.pixel_data;

    for (size_t y = 0; y < rsurface->height; y++) {
        for (size_t x = 0; x < rsurface->width; x++) {
            size_t i = (y * rsurface->width) + x;

            if (transparency && extern_data[i] == transparent_color) {
                buffer[i] = 0x00000000;
            } else {
                uint16_t red, green, blue;
                EXTRACT_RGB565(extern_data[i], &red, &green, &blue);

                buffer[i] = RGB565_TO_RGBA8888(red, green, blue, 0xFF);
            }
        }
    }
    
    texture__upload(&rsurface->texture, rsurface->width, rsurface->height, COLOR_FORMAT__RGBA32, buffer);

    SDL_free(buffer);
    rsurface->locked = false;
}

static int compare_surfaces_by_id(void const* a, void const* b) {
    rsurface_id_t const* a_id = a;
    rsurface_id_t const* b_id = b;

    return (*a_id > *b_id) - (*a_id < *b_id);
}
