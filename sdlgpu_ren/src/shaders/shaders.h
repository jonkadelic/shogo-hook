#pragma once

#include <stddef.h>
#include <stdint.h>

#include <SDL3/SDL.h>

typedef struct shader_init_data {
    size_t const* src_len; void const* src;
    char const* entry_point;
    size_t num_samplers;
    size_t num_storage_textures;
    size_t num_storage_buffers;
    size_t num_uniform_buffers;
} shader_init_data_t;

typedef enum shader_id {
    SHADER_ID__BLIT_2D,
    NUM_SHADER_IDS,
} shader_id_t;

typedef struct shader_def {
    shader_id_t id;

    shader_init_data_t vertex_data;
    shader_init_data_t fragment_data;

    size_t num_vertex_buffers;
    SDL_GPUVertexBufferDescription const* vertex_buffers;
    size_t num_vertex_attributes;
    SDL_GPUVertexAttribute const* vertex_attributes;
} shader_def_t;

extern shader_def_t const SHADER_DEFS[];