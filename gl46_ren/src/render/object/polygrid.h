#pragma once

#include <lithtech/lithtech.h>

#include "render/mesh.h"
#include "render/tessellator.h"

typedef struct object_data object_data_t;

typedef struct polygrid {
    uint32_t width, height;
    mesh_t mesh; bool mesh_init;
    GLuint gl_offsets_ssbo;
} polygrid_t;

void polygrid__draw(object_data_t* self, tessellator_t* tessellator, DObject_t const* object);
void polygrid__cleanup(object_data_t* self);
