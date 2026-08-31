#pragma once

#include <stdint.h>

#include <glad/gl.h>

#include "render/texture.h"
#include "util/rect_buffer.h"

typedef struct surface {
    size_t idx;
    int32_t width, height;
    bool locked;
    texture_t texture;
    rect_buffer_t buffer;
} surface_t;

typedef struct surface_manager {
    size_t surfaces_capacity;
    surface_t** surfaces;
} surface_manager_t;

bool surface_manager__init(surface_manager_t* self);
void surface_manager__cleanup(surface_manager_t* self);

surface_t* surface_manager__create_surface(surface_manager_t* self, int32_t width, int32_t height);
void surface_manager__delete_surface(surface_manager_t* self, size_t idx);

void surface__update_texture(surface_t* self, bool transparency, uint16_t transparent_color);
