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
    MainWorld_t const* world;
    tessellator_t* tessellator;
    shared_texture_manager_t* textures;

    size_t models_len;
    world_model_t* models;
} world_t;

bool world__init(
    world_t* self,
    SceneDesc_t const* scene,
    tessellator_t* tessellator,
    shared_texture_manager_t* textures
);
void world__cleanup(world_t* self);

void world__draw(world_t* self, SceneDesc_t const* scene);
