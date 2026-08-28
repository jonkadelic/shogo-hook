#include "./tessellator.h"

#include <assert.h>
#include <stdlib.h>

#include "util/debug.h"
#include "util/util.h"

#define BUFFER_ALLOC_LEN    (4)
#define VERTEX_ALLOC_LEN    (4)
#define INDEX_ALLOC_LEN     (4)

bool tessellator__init(tessellator_t* self, SDL_GPUDevice* device) {
    ZERO_INIT_STRUCT(self);

    self->device = device;

    return true;
}

void tessellator__cleanup(tessellator_t* self) {
    for (size_t i = 0; i < self->buffers_capacity; i++) {
        SDL_free(self->buffers[i]->vertices);
        SDL_free(self->buffers[i]->indices);
        SDL_free(self->buffers[i]);
    }

    SDL_free(self->buffers);

    self->buffers_capacity = 0;
    self->buffers = nullptr;

    SDL_ReleaseGPUTransferBuffer(self->device, self->tx_buffer);
    self->tx_buffer_size = 0;
    self->tx_buffer = nullptr;

    self->device = nullptr;
}

tessellator_buffer_t* tessellator__start(tessellator_t* self, mesh_t* mesh) {
    tessellator_buffer_t* out = nullptr;

    for (size_t i = 0; i < self->buffers_capacity; i++) {
        assert(self->buffers[i] != nullptr);

        if (self->buffers[i]->state == TESSELLATOR_BUFFER_STATE__FREE) {
            out = self->buffers[i];
            break;
        }
    }

    if (out == nullptr) {
        size_t new_buffers_capacity = self->buffers_capacity + BUFFER_ALLOC_LEN;
        tessellator_buffer_t** new_buffers = SDL_realloc(self->buffers, sizeof(tessellator_buffer_t*) * new_buffers_capacity);
        if (new_buffers == nullptr) {
            SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "Failed to allocate %zu tessellator buffers.", new_buffers_capacity);
            return nullptr;
        }
        
        for (size_t i = self->buffers_capacity; i < new_buffers_capacity; i++) {
            new_buffers[i] = SDL_calloc(1, sizeof(tessellator_buffer_t));
            if (new_buffers[i] == nullptr) {
                SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "Failed to allocate tessellator buffer.");
                self->buffers_capacity = self->buffers_capacity + i;
                self->buffers = new_buffers;
                return nullptr;
            }
        }

        out = new_buffers[self->buffers_capacity];
        self->buffers_capacity = new_buffers_capacity;
        self->buffers = new_buffers;
    }

    out->mesh = mesh;
    out->state = TESSELLATOR_BUFFER_STATE__TAKEN;

    return out;
}

bool tessellator__needs_flush(tessellator_t* self) {
    for (size_t i = 0; i < self->buffers_capacity; i++) {
        if (self->buffers[i]->state == TESSELLATOR_BUFFER_STATE__LOCKED) {
            return true;
        }
    }

    return false;
}

void tessellator__flush_to_gpu(tessellator_t* self, SDL_GPUCommandBuffer* cmd_buffer, SDL_GPUCopyPass* opt_copy_pass) {
    SDL_GPUCopyPass* copy_pass = opt_copy_pass;

    bool needs_flush = false;

    // Get required transfer buffer length
    size_t new_tx_buffer_size = 0;
    for (size_t i = 0; i < self->buffers_capacity; i++) {
        tessellator_buffer_t const* buffer = self->buffers[i];

        if (buffer->state == TESSELLATOR_BUFFER_STATE__LOCKED) {
            size_t vertices_size = sizeof(vertex_t) * buffer->vertices_len;
            size_t indices_size = sizeof(index_t) * buffer->indices_len;

            new_tx_buffer_size += vertices_size + indices_size; 

            needs_flush = true;
        }
    }
    if (!needs_flush) {
        return;
    }

    // Ensure mesh buffers are sufficient
    for (size_t i = 0; i < self->buffers_capacity; i++) {
        tessellator_buffer_t const* buffer = self->buffers[i];

        if (buffer->state == TESSELLATOR_BUFFER_STATE__LOCKED) {
            mesh_t* mesh = buffer->mesh;

            // Update vertex buffer
            if (mesh->vertices_capacity < buffer->vertices_len) {
                SDL_ReleaseGPUBuffer(mesh->device, mesh->vertices);
                
                SDL_GPUBufferCreateInfo buffer_info = {
                    .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
                    .size = buffer->vertices_len * sizeof(vertex_t),
                };
                mesh->vertices = SDL_CreateGPUBuffer(mesh->device, &buffer_info);
                if (mesh->vertices == nullptr) {
                    SDL_LogError(SDL_LOG_CATEGORY_GPU, "Failed to create vertex buffer of %zu vertices: %s", buffer->vertices_len, SDL_GetError());
                    return;
                }

                mesh->vertices_capacity = buffer->vertices_len;
            }

            // Update index buffer
            if (mesh->indices_capacity < buffer->indices_len) {
                SDL_ReleaseGPUBuffer(mesh->device, mesh->indices);

                SDL_GPUBufferCreateInfo buffer_info = {
                    .usage = SDL_GPU_BUFFERUSAGE_INDEX,
                    .size = buffer->indices_len * sizeof(index_t),
                };
                mesh->indices = SDL_CreateGPUBuffer(mesh->device, &buffer_info);
                if (mesh->indices == nullptr) {
                    SDL_LogError(SDL_LOG_CATEGORY_GPU, "Failed to create index buffer of %zu indices: %s", buffer->indices_len, SDL_GetError());
                    return;
                }

                mesh->indices_capacity = buffer->indices_len;
            }
        }
    }

    // Re-create transfer buffer if needed
    if (new_tx_buffer_size > self->tx_buffer_size) {
        SDL_GPUTransferBufferCreateInfo info = {
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .size = new_tx_buffer_size,
        };
        SDL_GPUTransferBuffer* new_tx_buffer = SDL_CreateGPUTransferBuffer(self->device, &info);
        if (new_tx_buffer == nullptr) {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "Failed to create transfer buffer of %zu bytes: %s", new_tx_buffer_size, SDL_GetError());
            goto end;
        }

        SDL_ReleaseGPUTransferBuffer(self->device, self->tx_buffer);

        self->tx_buffer_size = new_tx_buffer_size;
        self->tx_buffer = new_tx_buffer;
    }

    // Begin copy pass
    if (copy_pass == nullptr) {
        copy_pass = SDL_BeginGPUCopyPass(cmd_buffer);
        if (copy_pass == nullptr) {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "Failed to begin copy pass: %s", SDL_GetError());
            goto end;
        }
    }

    // Map transfer buffer
    uint8_t* tx_buffer_data = SDL_MapGPUTransferBuffer(self->device, self->tx_buffer, false);
    if (tx_buffer_data == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_GPU, "Failed to map transfer buffer: %s", SDL_GetError());
        goto end;
    }
    
    // Iterate over transfers
    size_t tx_buffer_offset = 0;
    for (size_t i = 0; i < self->buffers_capacity; i++) {
        tessellator_buffer_t* buffer = self->buffers[i];

        if (buffer->state == TESSELLATOR_BUFFER_STATE__LOCKED) {
            mesh_t* mesh = buffer->mesh;

            SDL_GPUTransferBufferLocation copy_src = {
                .transfer_buffer = self->tx_buffer,
            };
            SDL_GPUBufferRegion copy_dst;

            // Copy vertex data to transfer buffer
            size_t vertices_size = sizeof(vertex_t) * buffer->vertices_len;
            SDL_assert(tx_buffer_offset + vertices_size <= self->tx_buffer_size);
            SDL_memcpy(tx_buffer_data + tx_buffer_offset, buffer->vertices, vertices_size);
            
            // Buffer upload command for vertex data
            copy_src.offset = tx_buffer_offset;
            copy_dst.buffer = mesh->vertices;
            copy_dst.size = vertices_size;
            SDL_UploadToGPUBuffer(copy_pass, &copy_src, &copy_dst, false);

            mesh->vertices_len = buffer->vertices_len;
            tx_buffer_offset += vertices_size;

            // Copy index data to transfer buffer
            size_t indices_size = sizeof(index_t) * buffer->indices_len;
            SDL_assert(tx_buffer_offset + indices_size <= self->tx_buffer_size);
            SDL_memcpy(tx_buffer_data + tx_buffer_offset, buffer->indices, indices_size);
            
            // Buffer upload command for index data
            copy_src.offset = tx_buffer_offset;
            copy_dst.buffer = mesh->indices;
            copy_dst.size = indices_size;
            SDL_UploadToGPUBuffer(copy_pass, &copy_src, &copy_dst, false);

            mesh->indices_len = buffer->indices_len;
            tx_buffer_offset += indices_size;

            buffer->state = TESSELLATOR_BUFFER_STATE__FREE;
        }
    }

    // Unmap transfer buffer
    SDL_UnmapGPUTransferBuffer(self->device, self->tx_buffer);

end:
    if (copy_pass != opt_copy_pass) {
        SDL_EndGPUCopyPass(copy_pass);
    }
}

void tessellator_buffer__append_vertex(tessellator_buffer_t* self, vertex_t const* vertex) {
    tessellator_buffer__append_vertices(self, 1, vertex);
}

void tessellator_buffer__append_vertices(tessellator_buffer_t* self, size_t num_vertices, vertex_t const vertices[static num_vertices]) {
    if (self->state != TESSELLATOR_BUFFER_STATE__TAKEN) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Tried to append vertex to non-taken tessellator buffer.");
        return;
    }

    // Ensure buffer is large enough to fit new vertices
    size_t vertex_index = self->vertices_len;
    if (self->vertices_capacity < self->vertices_len + num_vertices) {
        size_t new_vertices_capacity = self->vertices_capacity + SDL_max(VERTEX_ALLOC_LEN, num_vertices);
        vertex_t* new_vertices = SDL_realloc(self->vertices, sizeof(vertex_t) * new_vertices_capacity);
        if (new_vertices == nullptr) {
            SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "Failed to allocate %zu vertices in tessellator.", new_vertices_capacity);
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

void tessellator_buffer__append_index(tessellator_buffer_t* self, index_t index) {
    tessellator_buffer__append_indices(self, 1, &index);
}

void tessellator_buffer__append_indices(tessellator_buffer_t* self, size_t num_indices, index_t const indices[static num_indices]) {
    if (self->state != TESSELLATOR_BUFFER_STATE__TAKEN) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Tried to append index to non-taken tessellator buffer.");
        return;
    }

    // Ensure buffer is large enough to fit new indices
    size_t index_index = self->indices_len;
    if (self->indices_len == self->indices_capacity) {
        size_t new_indices_capacity = self->indices_capacity + SDL_max(INDEX_ALLOC_LEN, num_indices);
        index_t* new_indices = SDL_realloc(self->indices, sizeof(index_t) * new_indices_capacity);
        if (new_indices == nullptr) {
            SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "Failed to allocate %zu indices in tessellator.", new_indices_capacity);
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

void tessellator_buffer__finish(tessellator_buffer_t* self) {
    self->state = TESSELLATOR_BUFFER_STATE__LOCKED;
}
