#pragma once

#include <stddef.h>

#include <glad/gl.h>

typedef struct vertex {
    float position[3];
    float color[4];
    float uv[2];
} vertex_t;

typedef uint32_t index_t;

typedef struct mesh {
    size_t vertex_size;
    GLuint gl_vertices_ssbo;
    GLuint gl_indices_ssbo;
    size_t index_count;
} mesh_t;

bool mesh__init(mesh_t* self, size_t vertex_size);
void mesh__cleanup(mesh_t* self);

void mesh__upload(
    mesh_t* self,
    size_t num_vertices, void const* vertices,
    size_t num_indices, index_t const indices[static num_indices]
);

void mesh__upload_with_usage(
    mesh_t* self,
    size_t num_vertices, void const* vertices,
    size_t num_indices, index_t const indices[static num_indices],
    GLenum usage
);

void mesh__draw(mesh_t* self);
