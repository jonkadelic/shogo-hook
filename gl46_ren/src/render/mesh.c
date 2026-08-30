#include "./mesh.h"

#include "logger.h"

#include <SDL3/SDL_assert.h>

#include "util/util.h"

static GLuint GL_EMPTY_VAO = 0;

bool mesh__init(mesh_t* self) {
    OBJECT_ZERO_INIT(self);

    if (GL_EMPTY_VAO == 0) {
        glGenVertexArrays(1, &GL_EMPTY_VAO);
        if (GL_EMPTY_VAO == 0) {
            LOG_ERROR("Failed to init empty VAO");
            goto err;
        }
    }

    glCreateBuffers(1, &self->gl_vertices_ssbo);
    if (self->gl_vertices_ssbo == 0) {
        LOG_ERROR("Failed to init vertex SSBO");
        goto err;
    }

    glCreateBuffers(1, &self->gl_indices_ssbo);
    if (self->gl_indices_ssbo == 0) {
        LOG_ERROR("Failed to init index SSBO");
        goto err;
    }

    return true;

err:
    mesh__cleanup(self);
    return false;
}

void mesh__cleanup(mesh_t* self) {
    glDeleteBuffers(1, &self->gl_indices_ssbo);
    self->gl_indices_ssbo = 0;
    glDeleteBuffers(1, &self->gl_vertices_ssbo);
    self->gl_vertices_ssbo = 0;
}

void mesh__upload(
    mesh_t* self,
    size_t num_vertices, vertex_t const vertices[static num_vertices],
    size_t num_indices, index_t const indices[static num_indices]
) {
    mesh__upload_with_usage(
        self,
        num_vertices, vertices,
        num_indices, indices,
        GL_STATIC_DRAW
    );
}

void mesh__upload_with_usage(
    mesh_t* self,
    size_t num_vertices, vertex_t const vertices[static num_vertices],
    size_t num_indices, index_t const indices[static num_indices],
    GLenum usage
) {
    SDL_assert(self->gl_vertices_ssbo != 0 && self->gl_indices_ssbo != 0);

    glNamedBufferData(self->gl_vertices_ssbo, num_vertices * sizeof(vertex_t), vertices, usage);
    glNamedBufferData(self->gl_indices_ssbo, num_indices * sizeof(index_t), indices, usage);
    self->index_count = num_indices;
}

void mesh__draw(mesh_t* self) {
    SDL_assert(self->gl_vertices_ssbo != 0 && self->gl_indices_ssbo != 0);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, self->gl_vertices_ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, self->gl_indices_ssbo);

    glBindVertexArray(GL_EMPTY_VAO);

    glDrawArrays(GL_TRIANGLES, 0, self->index_count);
}
