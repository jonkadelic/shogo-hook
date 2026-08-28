#pragma once

#include "shaders/shaders.h"
#include <SDL3/SDL.h>

typedef struct shader_pipeline {
    uint64_t config_hash;
    SDL_GPUGraphicsPipeline* pipeline;
} shader_pipeline_t;

typedef struct shader {
    SDL_GPUDevice* device;

    SDL_GPUShader* vertex_shader;
    SDL_GPUShader* fragment_shader;

    size_t num_vertex_buffers;
    SDL_GPUVertexBufferDescription* vertex_buffers;
    size_t num_vertex_attributes;
    SDL_GPUVertexAttribute* vertex_attributes;
    size_t num_color_targets;
    SDL_GPUColorTargetDescription* color_targets;

    size_t pipelines_capacity, pipelines_len;
    shader_pipeline_t* pipelines;
} shader_t;

typedef struct shader_config {
    bool blend;
} shader_config_t;

bool shader__init(
    shader_t* self,
    SDL_GPUDevice* device,
    size_t vertex_src_len, void const* vertex_src,
    size_t fragment_src_len, void const* fragment_src,
    size_t num_vertex_buffers, SDL_GPUVertexBufferDescription const* vertex_buffers,
    size_t num_vertex_attributes, SDL_GPUVertexAttribute const* vertex_attributes,
    size_t num_color_targets, SDL_GPUColorTargetDescription const* color_targets
);
bool shader__init_from_def(
    shader_t* self,
    SDL_GPUDevice* device,
    shader_def_t const* shader_def,
    size_t num_color_targets, SDL_GPUColorTargetDescription const* color_targets
);
void shader__cleanup(shader_t* self);

bool shader__bind(shader_t* self, shader_config_t config, SDL_GPURenderPass* render_pass);
