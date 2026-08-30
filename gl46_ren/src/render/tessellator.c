#include "./tessellator.h"

#include <SDL3/SDL_stdinc.h>

#include "logger.h"
#include "util/util.h"

#define VERTEX_ALLOC_LEN    (4)
#define INDEX_ALLOC_LEN     (4)

bool tessellator__init(tessellator_t* self) {
    OBJECT_ZERO_INIT(self);

    return true;
}

void tessellator__cleanup(tessellator_t* self) {
    SDL_free(self->indices); self->indices = nullptr;
    SDL_free(self->vertices); self->vertices = nullptr;
}

void tessellator__append_vertex(tessellator_t* self, vertex_t const* vertex) {
    tessellator__append_vertices(self, 1, vertex);
}

void tessellator__append_vertices(tessellator_t* self, size_t num_vertices, vertex_t const vertices[static num_vertices]) {
    // Ensure buffer is large enough to fit new vertices
    size_t vertex_index = self->vertices_len;
    if (self->vertices_capacity < self->vertices_len + num_vertices) {
        size_t new_vertices_capacity = self->vertices_capacity + SDL_max(VERTEX_ALLOC_LEN, num_vertices);
        vertex_t* new_vertices = SDL_realloc(self->vertices, sizeof(vertex_t) * new_vertices_capacity);
        if (new_vertices == nullptr) {
            LOG_ERROR("Failed to allocate %zu vertices in tessellator.", new_vertices_capacity);
            return;
        }

        self->vertices_capacity = new_vertices_capacity;
        self->vertices = new_vertices;
    }

    for (size_t i = 0; i < num_vertices; i++) {
        self->vertices[vertex_index + i] = vertices[i];
    }

    self->vertices_len += num_vertices;
}

void tessellator__append_index(tessellator_t* self, index_t index) {
    tessellator__append_indices(self, 1, &index);
}

void tessellator__append_indices(tessellator_t* self, size_t num_indices, index_t const indices[static num_indices]) {
    // Ensure buffer is large enough to fit new indices
    size_t index_index = self->indices_len;
    if (self->indices_len == self->indices_capacity) {
        size_t new_indices_capacity = self->indices_capacity + SDL_max(INDEX_ALLOC_LEN, num_indices);
        index_t* new_indices = SDL_realloc(self->indices, sizeof(index_t) * new_indices_capacity);
        if (new_indices == nullptr) {
            LOG_ERROR("Failed to allocate %zu indices in tessellator.", new_indices_capacity);
            return;
        }

        self->indices_capacity = new_indices_capacity;
        self->indices = new_indices;
    }

    for (size_t i = 0; i < num_indices; i++) {
        self->indices[index_index + i] = indices[i];
    }

    self->indices_len += num_indices;
}

void tessellator__upload_and_reset(tessellator_t* self, mesh_t* mesh) {
    tessellator__upload_and_reset_with_usage(self, mesh, GL_STATIC_DRAW);
}

void tessellator__upload_and_reset_with_usage(tessellator_t* self, mesh_t* mesh, GLenum usage) {
    mesh__upload_with_usage(mesh, self->vertices_len, self->vertices, self->indices_len, self->indices, usage);

    self->vertices_len = 0;
    self->indices_len = 0;
}
