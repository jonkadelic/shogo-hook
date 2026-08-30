#pragma once

#include <stdint.h>

#include "texture/textures.h"

typedef struct surface {
    size_t idx;
    uint32_t width, height;
    bool locked;
    texture_id_t texture;
    uint16_t extern_data[];
} surface_t;

typedef struct surface_manager {
    texture_manager_t* textures;

    size_t surfaces_capacity;
    surface_t** surfaces;
} surface_manager_t;

bool surface_manager__init(surface_manager_t* self, texture_manager_t* textures);
void surface_manager__cleanup(surface_manager_t* self);

surface_t* surface_manager__create_surface(surface_manager_t* self, uint32_t width, uint32_t height);
void surface_manager__delete_surface(surface_manager_t* self, size_t idx);
