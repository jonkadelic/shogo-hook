#include "./surfaces.h"
#include "logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL3/SDL.h>

#define SURFACE_ALLOC_NUM (4)

bool surface_manager__init(surface_manager_t* self) {
    *self = (surface_manager_t) { 0 };

    return true;
}

void surface_manager__cleanup(surface_manager_t* self) {
    for (size_t i = 0; i < self->surfaces_capacity; i++) {
        if (self->surfaces[i] != nullptr) {
            texture__cleanup(&self->surfaces[i]->texture);
            SDL_free(self->surfaces[i]);
        }
    }

    SDL_free(self->surfaces);
    self->surfaces = nullptr;
    self->surfaces_capacity = 0;
}

surface_t* surface_manager__create_surface(surface_manager_t* self, int32_t width, int32_t height) {
    if (width <= 0 || height <= 0) {
        return nullptr;
    }

    surface_t** surface = nullptr;
    size_t surface_idx = 0;
    for (size_t i = 0; i < self->surfaces_capacity; i++) {
        if (self->surfaces[i] == nullptr) {
            surface = &self->surfaces[i];
            surface_idx = i;
            break;
        }
    }

    if (surface == nullptr) {
        size_t new_surfaces_len = self->surfaces_capacity + SURFACE_ALLOC_NUM;
        surface_t** new_surfaces = SDL_realloc(self->surfaces, sizeof(surface_t*) * new_surfaces_len);
        if (new_surfaces == nullptr) {
            LOG_ERROR("Failed to reallocate %zu surfaces", new_surfaces_len);
            goto err;
        }

        for (size_t i = self->surfaces_capacity; i < new_surfaces_len; i++) {
            new_surfaces[i] = nullptr;
        }

        surface = &new_surfaces[self->surfaces_capacity];
        surface_idx = self->surfaces_capacity;

        self->surfaces_capacity = new_surfaces_len;
        self->surfaces = new_surfaces;
    }

    *surface = SDL_calloc(1, sizeof(surface_t));
    if (*surface == nullptr) {
        LOG_ERROR("Failed to allocate surface");
        goto err;
    }

    (*surface)->idx = surface_idx;
    (*surface)->width = width;
    (*surface)->height = height;

    if (!texture__init(&(*surface)->texture)) {
        LOG_ERROR("Failed to create surface texture");
        goto err;
    }

    if (!rect_buffer__init(&(*surface)->buffer, width, height, 2)) {
        LOG_ERROR("Failed to create surface buffer");
        goto err;
    }

    return *surface;

err:
    if (*surface != nullptr) {
        rect_buffer__cleanup(&(*surface)->buffer);
        texture__cleanup(&(*surface)->texture);
    }
    SDL_free(*surface);
    return nullptr;
}

void surface_manager__delete_surface(surface_manager_t* self, size_t idx) {
    if (idx >= self->surfaces_capacity) return;

    auto surface = self->surfaces[idx];
    if (surface == nullptr) return;

    rect_buffer__cleanup(&surface->buffer);
    texture__cleanup(&surface->texture);
    SDL_free(self->surfaces[idx]);
    self->surfaces[idx] = nullptr;
}

void surface__update_texture(surface_t* self, bool transparency, uint16_t transparent_color) {
    if (self->width == 0 || self->height == 0) {
        return;
    }

    if (!(self->locked || self->texture.gl_texture == 0)) {
        return;
    }

    uint32_t* buffer = SDL_malloc(self->width * self->height * sizeof(uint32_t));
    if (buffer == nullptr) {
        LOG_ERROR("Failed to allocate copy buffer for surface");
        return;
    }

    uint16_t const* extern_data = (uint16_t const*) self->buffer.data;

    for (size_t y = 0; y < self->height; y++) {
        for (size_t x = 0; x < self->width; x++) {
            size_t i = (y * self->width) + x;

            uint8_t red = (extern_data[i] & 0xF800) >> 11;
            uint8_t green = (extern_data[i] & 0x07C0) >> 6;
            uint8_t blue = (extern_data[i] & 0x003F) >> 0;

            if (transparency && extern_data[i] == transparent_color) {
                buffer[i] = 0x00000000;
            } else {
                buffer[i] = 0xFF000000 |
                            (red << 3) |
                            ((green << 3) << 8) |
                            ((blue << 3) << 16);
            }
        }

        texture__upload(&self->texture, self->width, self->height, buffer);
    }

    SDL_free(buffer);
    self->locked = false;
}
