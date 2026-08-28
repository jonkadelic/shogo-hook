#pragma once

#include <stddef.h>
#include <stdint.h>

#include <SDL3/SDL.h>

typedef enum shader_id {
    SHADER_ID__PASSTHROUGH,
    NUM_SHADER_IDS,
} shader_id_t;

typedef struct shader_def {
    shader_id_t id;

    size_t const* vertex_src_len;
    void const* vertex_src;
    size_t const* fragment_src_len;
    void const* fragment_src;

    size_t num_vertex_buffers;
    SDL_GPUVertexBufferDescription const* vertex_buffers;
    size_t num_vertex_attributes;
    SDL_GPUVertexAttribute const* vertex_attributes;
} shader_def_t;

extern shader_def_t const SHADER_DEFS[NUM_SHADER_IDS];