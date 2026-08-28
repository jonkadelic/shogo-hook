#pragma once

#include <stddef.h>
#include <stdint.h>

#include "mesh/mesh.h"

typedef uint16_t index_t;

typedef enum tessellator_buffer_state {
    TESSELLATOR_BUFFER_STATE__FREE,
    TESSELLATOR_BUFFER_STATE__TAKEN,
    TESSELLATOR_BUFFER_STATE__LOCKED,
} tessellator_buffer_state_t;

typedef struct tessellator_buffer {
    tessellator_buffer_state_t state;
    mesh_t* mesh;

    size_t vertices_len, vertices_capacity;
    vertex_t* vertices;
    size_t indices_len, indices_capacity;
    index_t* indices;
} tessellator_buffer_t;

typedef struct tessellator {
    SDL_GPUDevice* device;

    size_t tx_buffer_size;
    SDL_GPUTransferBuffer* tx_buffer;
    
    size_t buffers_capacity;
    tessellator_buffer_t** buffers; // double-pointer so that realloc() maintains addresses of buffers
} tessellator_t;

bool tessellator__init(tessellator_t* self, SDL_GPUDevice* device);
void tessellator__cleanup(tessellator_t* self);

tessellator_buffer_t* tessellator__start(tessellator_t* self, mesh_t* mesh);

bool tessellator__needs_flush(tessellator_t* self);
void tessellator__flush_to_gpu(tessellator_t* self, SDL_GPUCommandBuffer* cmd_buffer, SDL_GPUCopyPass* opt_copy_pass);

void tessellator_buffer__append_vertex(tessellator_buffer_t* self, vertex_t const* vertex);
void tessellator_buffer__append_vertices(tessellator_buffer_t* self, size_t num_vertices, vertex_t const vertices[static num_vertices]);
void tessellator_buffer__append_index(tessellator_buffer_t* self, index_t index);
void tessellator_buffer__append_indices(tessellator_buffer_t* self, size_t num_indices, index_t const indices[static num_indices]);

void tessellator_buffer__finish(tessellator_buffer_t* self);

