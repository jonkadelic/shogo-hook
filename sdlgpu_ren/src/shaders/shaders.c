#include "./shaders.h"

#include "compiled_shaders.h"
#include "mesh/mesh.h"

shader_def_t const SHADER_DEFS[] = {
    [SHADER_ID__BLIT_2D] = {
        .id = SHADER_ID__BLIT_2D,
        .vertex_data = {
            .src_len = &blit_2d_slang_spv_len,
            .src = blit_2d_slang_spv,
            .num_uniform_buffers = 1,
        },
        .fragment_data = {
            .src_len = &blit_2d_slang_spv_len,
            .src = blit_2d_slang_spv,
            .num_samplers = 1,
            .num_storage_textures = 1,
        },
        .num_vertex_buffers = 1,
        .vertex_buffers = (SDL_GPUVertexBufferDescription[]) {
            (SDL_GPUVertexBufferDescription) {
                .slot = 0,
                .pitch = sizeof(vertex_t),
                .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
                .instance_step_rate = 0,
            }
        },
        .num_vertex_attributes = 2,
        .vertex_attributes = (SDL_GPUVertexAttribute[]) {
            // x, y, z
            (SDL_GPUVertexAttribute) {
                .location = 0,
                .buffer_slot = 0,
                .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
                .offset = offsetof(vertex_t, x),
            },
            // r, g, b, a
            (SDL_GPUVertexAttribute) {
                .location = 1,
                .buffer_slot = 0,
                .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
                .offset = offsetof(vertex_t, r),
            }
        }
    }
};
static_assert(sizeof(SHADER_DEFS) == sizeof(shader_def_t) * NUM_SHADER_IDS);