#include "./surfaces.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/util.h"

#define SURFACE_ALLOC_NUM (4)

bool surface_manager__init(surface_manager_t* self, texture_manager_t* textures) {
    ZERO_INIT_STRUCT(self);

    self->textures = textures;

    return true;
}

void surface_manager__cleanup(surface_manager_t* self) {
    for (size_t i = 0; i < self->surfaces_capacity; i++) {
        // glDeleteTextures(1, &self->surfaces[i]->glTexture);
        SDL_free(self->surfaces[i]);
    }

    SDL_free(self->surfaces);
    self->surfaces = 0;
}

surface_t* surface_manager__create_surface(surface_manager_t* self, uint32_t width, uint32_t height) {
    surface_t** surface = nullptr;

    // Check if existing surface slot can be reused
    size_t surface_idx = 0;
    for (size_t i = 0; i < self->surfaces_capacity; i++) {
        if (self->surfaces[i] == nullptr) {
            surface = &self->surfaces[i];
            surface_idx = i;
            break;
        }
    }

    // If no surface, resize surface list and initialize
    if (surface == nullptr) {
        size_t new_surfaces_capacity = self->surfaces_capacity + SURFACE_ALLOC_NUM;
        surface_t** new_surfaces = SDL_realloc(self->surfaces, sizeof(surface_t*) * new_surfaces_capacity);
        if (new_surfaces == nullptr) {
            SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "Failed to allocate %zu surfaces", new_surfaces_capacity);
            return nullptr;
        }

        for (size_t i = self->surfaces_capacity; i < new_surfaces_capacity; i++) {
            new_surfaces[i] = nullptr;
        }

        surface = &new_surfaces[self->surfaces_capacity];
        surface_idx = self->surfaces_capacity;

        self->surfaces_capacity = new_surfaces_capacity;
        self->surfaces = new_surfaces;
    }

    // Allocate surface and initialize
    *surface = SDL_calloc(1, sizeof(surface_t) + width * height * sizeof(uint16_t));
    if (*surface == nullptr) {
        return nullptr;
    }
    (*surface)->idx = surface_idx;
    (*surface)->width = width;
    (*surface)->height = height;

    return *surface;
}

void surface_manager__delete_surface(surface_manager_t* self, size_t idx) {
    if (idx >= self->surfaces_capacity) {
        return;
    }

    texture_manager__delete_texture(self->textures, self->surfaces[idx]->texture);

    SDL_free(self->surfaces[idx]);
    self->surfaces[idx] = nullptr;
}
