#pragma once

#include <stddef.h>

#include "render/mesh.h"

typedef struct tessellator {
    size_t vertex_size;
    size_t vertices_len, vertices_capacity;
    void* vertices;
    size_t indices_len, indices_capacity;
    index_t* indices;
} tessellator_t;

bool tessellator__init(tessellator_t* self, size_t vertex_size);
void tessellator__cleanup(tessellator_t* self);

void tessellator__append_vertex(tessellator_t* self, void const* vertex);
void tessellator__append_vertices(tessellator_t* self, size_t num_vertices, void const* vertices);
void tessellator__append_index(tessellator_t* self, index_t index);
void tessellator__append_indices(tessellator_t* self, size_t num_indices, index_t const indices[static num_indices]);

void tessellator__upload_and_reset(tessellator_t* self, mesh_t* mesh);
void tessellator__upload_and_reset_with_usage(tessellator_t* self, mesh_t* mesh, GLenum usage);