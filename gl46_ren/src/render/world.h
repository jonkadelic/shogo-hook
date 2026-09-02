#pragma once

#include <stdint.h>

#include <lithtech/lithtech.h>

#include "render/mesh.h"
#include "render/shared_textures.h"
#include "render/tessellator.h"
#include "render/texture.h"

typedef struct world_model {
    mesh_t mesh;
    texture_t* texture;
} world_model_t;

typedef struct world {
    size_t models_len;
    world_model_t* models;
} world_t;

bool world__init(
    world_t* self,
    MainWorld_t const* main_world,
    tessellator_t* tessellator,
    shared_texture_manager_t* textures
);
void world__cleanup(world_t* self);

void world__draw(world_t* self);
