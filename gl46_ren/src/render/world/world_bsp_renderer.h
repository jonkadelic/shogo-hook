#pragma once

#include <lithtech/lithtech.h>

#include "render/mesh.h"
#include "render/texture.h"

typedef struct world_bsp_model {
    mesh_t mesh;
    texture_t* texture;
} world_bsp_model_t;

typedef struct world_bsp_renderer {
    WorldBsp_t const* world_bsp;

    size_t models_len;
    world_bsp_model_t* models;
} world_bsp_renderer_t;

bool world_bsp_renderer__init(
    world_bsp_renderer_t* self,
    WorldBsp_t const* world_bsp
);
void world_bsp_renderer__cleanup(world_bsp_renderer_t* self);

void world_bsp_renderer__draw(world_bsp_renderer_t* self, WorldBsp_t const* world_bsp);
