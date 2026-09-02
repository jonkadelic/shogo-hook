#include "./rsurfaces.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL3/SDL.h>

#include "logger.h"
#include "util/util.h"

#define RSURFACE_ALLOC_NUM (4)

bool rsurface_manager__init(rsurface_manager_t* self) {
    *self = (rsurface_manager_t) { 0 };

    return true;
}

void rsurface_manager__cleanup(rsurface_manager_t* self) {
    for (size_t i = 0; i < self->rsurfaces_capacity; i++) {
        if (self->rsurfaces[i] != nullptr) {
            texture__cleanup(&self->rsurfaces[i]->texture);
            SDL_free(self->rsurfaces[i]);
        }
    }

    SDL_free(self->rsurfaces);
    self->rsurfaces = nullptr;
    self->rsurfaces_capacity = 0;
}

rsurface_t* rsurface_manager__create_rsurface(rsurface_manager_t* self, int32_t width, int32_t height) {
    if (width <= 0 || height <= 0) {
        return nullptr;
    }

    rsurface_t** rsurface = nullptr;
    size_t rsurface_idx = 0;
    for (size_t i = 0; i < self->rsurfaces_capacity; i++) {
        if (self->rsurfaces[i] == nullptr) {
            rsurface = &self->rsurfaces[i];
            rsurface_idx = i;
            break;
        }
    }

    if (rsurface == nullptr) {
        size_t new_rsurfaces_len = self->rsurfaces_capacity + RSURFACE_ALLOC_NUM;
        rsurface_t** new_rsurfaces = SDL_realloc(self->rsurfaces, sizeof(rsurface_t*) * new_rsurfaces_len);
        if (new_rsurfaces == nullptr) {
            LOG_ERROR("Failed to reallocate %zu rsurfaces", new_rsurfaces_len);
            goto err;
        }

        for (size_t i = self->rsurfaces_capacity; i < new_rsurfaces_len; i++) {
            new_rsurfaces[i] = nullptr;
        }

        rsurface = &new_rsurfaces[self->rsurfaces_capacity];
        rsurface_idx = self->rsurfaces_capacity;

        self->rsurfaces_capacity = new_rsurfaces_len;
        self->rsurfaces = new_rsurfaces;
    }

    *rsurface = SDL_calloc(1, sizeof(rsurface_t));
    if (*rsurface == nullptr) {
        LOG_ERROR("Failed to allocate rsurface");
        goto err;
    }

    (*rsurface)->idx = rsurface_idx;
    (*rsurface)->width = width;
    (*rsurface)->height = height;

    if (!texture__init(&(*rsurface)->texture, nullptr)) {
        LOG_ERROR("Failed to create rsurface texture");
        goto err;
    }

    if (!pixel_buffer__init(&(*rsurface)->buffer, width, height, COLOR_FORMAT__RGBA32)) {
        LOG_ERROR("Failed to create rsurface buffer");
        goto err;
    }

    return *rsurface;

err:
    if (rsurface != nullptr && *rsurface != nullptr) {
        pixel_buffer__cleanup(&(*rsurface)->buffer);
        texture__cleanup(&(*rsurface)->texture);
    }
    SDL_free(*rsurface);
    return nullptr;
}

void rsurface_manager__delete_rsurface(rsurface_manager_t* self, size_t idx) {
    if (idx >= self->rsurfaces_capacity) return;

    auto rsurface = self->rsurfaces[idx];
    if (rsurface == nullptr) return;

    pixel_buffer__cleanup(&rsurface->buffer);
    texture__cleanup(&rsurface->texture);
    SDL_free(self->rsurfaces[idx]);
    self->rsurfaces[idx] = nullptr;
}

void rsurface__update_texture(rsurface_t* self, bool transparency, uint16_t transparent_color) {
    if (self->width == 0 || self->height == 0) {
        return;
    }

    if (!(self->locked || self->texture.gl_texture == 0)) {
        return;
    }

    uint32_t* buffer = SDL_malloc(self->width * self->height * sizeof(uint32_t));
    if (buffer == nullptr) {
        LOG_ERROR("Failed to allocate copy buffer for rsurface");
        return;
    }

    uint16_t const* extern_data = (uint16_t const*) self->buffer.pixel_data;

    for (size_t y = 0; y < self->height; y++) {
        for (size_t x = 0; x < self->width; x++) {
            size_t i = (y * self->width) + x;

            if (transparency && extern_data[i] == transparent_color) {
                buffer[i] = 0x00000000;
            } else {
                uint16_t red, green, blue;
                EXTRACT_RGB565(extern_data[i], &red, &green, &blue);

                buffer[i] = RGB565_TO_RGBA8888(red, green, blue, 0xFF);
            }
        }

        texture__upload(&self->texture, self->width, self->height, COLOR_FORMAT__RGBA32, buffer);
    }

    SDL_free(buffer);
    self->locked = false;
}
