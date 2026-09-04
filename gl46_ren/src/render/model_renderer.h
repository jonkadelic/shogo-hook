#pragma once

#include <stddef.h>

#include <lithtech/lithtech.h>

#include "render/mesh.h"
#include "render/texture.h"

typedef struct model_vertex {
    float position[3];
    float color[4];
    float uv[2];
    uint32_t node_idx;
} model_vertex_t;

typedef struct model_renderer {
    texture_t* texture;
    size_t num_meshes;
    mesh_t* meshes;
    GLuint gl_node_matrix_ssbo;
} model_renderer_t;

bool model_renderer__init(model_renderer_t* self, ModelInstance_t const* model_instance);
void model_renderer__cleanup(model_renderer_t* self);

void model_renderer__draw(model_renderer_t* self, ModelInstance_t const* model_instance);
