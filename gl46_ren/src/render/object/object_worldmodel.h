#pragma once

#include "render/tessellator.h"
#include "render/world/world_bsp_renderer.h"

typedef struct object_data object_data_t;

typedef struct object_worldmodel {
    bool renderer_init;
    world_bsp_renderer_t renderer;
} object_worldmodel_t;

void object_worldmodel__draw(object_data_t* self, SceneDesc_t const* scene_desc, DObject_t const* object);
void object_worldmodel__cleanup(object_data_t* self);
