#pragma once

#include <lithtech/lithtech.h>

#include "render/mesh.h"
#include "render/tessellator.h"

typedef struct object_data object_data_t;

typedef struct object_polygrid {
    uint32_t width, height;
    mesh_t mesh; bool mesh_init;
    GLuint gl_offsets_ssbo;
    GLuint gl_colors_ssbo;
} object_polygrid_t;

void object_polygrid__draw(object_data_t* self, SceneDesc_t const* scene_desc, DObject_t const* object);
void object_polygrid__cleanup(object_data_t* self);
