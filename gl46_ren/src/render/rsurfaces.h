#pragma once

#include <stdint.h>

#include <glad/gl.h>

#include "render/texture.h"
#include "render/pixel_buffer.h"

typedef uint32_t rsurface_id_t;

#define RSURFACE_ID_INVALID ((rsurface_id_t) 0)

typedef struct rsurface {
    rsurface_id_t id;
    int32_t width, height;
    bool locked;
    texture_t texture;
    pixel_buffer_t buffer;
} rsurface_t;

typedef struct rsurface_manager {
    rsurface_id_t next_id;

    size_t rsurfaces_capacity;
    rsurface_t* rsurfaces;
} rsurface_manager_t;

bool rsurface_manager__init(rsurface_manager_t* self);
void rsurface_manager__cleanup(rsurface_manager_t* self);

rsurface_id_t rsurface_manager__create_rsurface(rsurface_manager_t* self, int32_t width, int32_t height);
void rsurface_manager__delete_rsurface(rsurface_manager_t* self, rsurface_id_t id);
rsurface_t* rsurface_manager__get_rsurface(rsurface_manager_t* self, rsurface_id_t id);

void rsurface_manager__update_texture(rsurface_manager_t* self, rsurface_id_t id, bool transparency, uint16_t transparent_color);
