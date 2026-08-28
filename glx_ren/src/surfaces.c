#include "./surfaces.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool surface_manager__init(surface_manager_t* self) {
    *self = (surface_manager_t) { 0 };

    return true;
}

void surface_manager__cleanup(surface_manager_t* self) {
    for (size_t i = 0; i < self->surfaces_len; i++) {
        glDeleteTextures(1, &self->surfaces[i]->glTexture);
        free(self->surfaces[i]);
    }

    free(self->surfaces);
}

surface_t* surface_manager__create_surface(surface_manager_t* self, int32_t width, int32_t height) {
    if (width <= 0 || height <= 0) {
        return nullptr;
    }

    surface_t** surface = nullptr;
    size_t surface_idx = 0;
    for (size_t i = 0; i < self->surfaces_len; i++) {
        if (self->surfaces[i] == nullptr) {
            surface = &self->surfaces[i];
            surface_idx = i;
            break;
        }
    }

    if (surface == nullptr) {
        size_t new_surfaces_len = self->surfaces_len == 0 ? 1 : self->surfaces_len * 2;
        surface_t** new_surfaces = realloc(self->surfaces, sizeof(surface_t*) * new_surfaces_len);
        if (new_surfaces == nullptr) {
            return nullptr;
        }

        for (size_t i = self->surfaces_len; i < new_surfaces_len; i++) {
            new_surfaces[i] = nullptr;
        }

        surface = &new_surfaces[self->surfaces_len];
        surface_idx = self->surfaces_len;

        self->surfaces_len = new_surfaces_len;
        self->surfaces = new_surfaces;
    }

    *surface = calloc(1, sizeof(surface_t) + width * height * sizeof(uint16_t));
    if (*surface == nullptr) {
        return nullptr;
    }

    (*surface)->idx = surface_idx;
    (*surface)->width = width;
    (*surface)->height = height;

    return *surface;
}

void surface_manager__delete_surface(surface_manager_t* self, size_t idx) {
    if (idx >= self->surfaces_len) {
        return;
    }

    GLuint texture = self->surfaces[idx]->glTexture;
    if (texture != 0) {
        glDeleteTextures(1, &texture);
    }
    free(self->surfaces[idx]);
    self->surfaces[idx] = nullptr;
}