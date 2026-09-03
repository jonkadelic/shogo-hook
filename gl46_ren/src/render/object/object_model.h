#pragma once

#include <stdint.h>

#include "render/model_renderer.h"
#include "render/tessellator.h"

typedef struct object_data object_data_t;

typedef struct object_model {
    bool renderer_init;
    model_renderer_t renderer;
} object_model_t;

void object_model__draw(object_data_t* self, SceneDesc_t const* scene_desc, DObject_t const* object);
void object_model__cleanup(object_data_t* self);
