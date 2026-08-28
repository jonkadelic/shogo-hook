#include "./shaders.h"

#include "compiled_shaders.h"
#include "mesh/mesh.h"

shader_def_t const SHADER_DEFS[] = {
    [SHADER_ID__PASSTHROUGH] = {
        .id = SHADER_ID__PASSTHROUGH,
        .vertex_src_len = &passthrough_vert_spv_len,
        .vertex_src = passthrough_vert_spv,
        .fragment_src_len = &passthrough_frag_spv_len,
        .fragment_src = passthrough_frag_spv,
        .num_vertex_buffers = 1,
        .vertex_buffers = (SDL_GPUVertexBufferDescription[]) {
            (SDL_GPUVertexBufferDescription) {
                .slot = 0,
                .pitch = sizeof(vertex_t),
                .input_rate = SDL_GPU_VERTEXINPUTRATE_INSTANCE,
                .instance_step_rate = 0,
            }
        },
        .num_vertex_attributes = 2,
        .vertex_attributes = (SDL_GPUVertexAttribute[]) {
            // x, y, z
            (SDL_GPUVertexAttribute) {
                .location = 0,
                .buffer_slot = 0,
                .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
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