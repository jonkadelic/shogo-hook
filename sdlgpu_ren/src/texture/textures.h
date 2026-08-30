#pragma once

#include <stdint.h>

#include <SDL3/SDL.h>

#include "texture/samplers.h"

#define TEXTURE_ID_INVALID  (0)

typedef uint32_t texture_id_t;

typedef struct texture {
    bool assigned;
    uint16_t gen;
    SDL_GPUTexture* texture;
    uint32_t width, height;
} texture_t;

typedef struct texture_upload {
    texture_id_t texture;
    void* data;
    size_t width, height;
} texture_upload_t;

typedef struct texture_manager {
    SDL_GPUDevice* device;

    sampler_manager_t samplers;

    size_t tx_buffer_size;
    SDL_GPUTransferBuffer* tx_buffer;

    size_t textures_capacity;
    texture_t* textures;

    size_t uploads_len, uploads_capacity;
    texture_upload_t* uploads;
} texture_manager_t;

bool texture_manager__init(texture_manager_t* self, SDL_GPUDevice* device);
void texture_manager__cleanup(texture_manager_t* self);

bool texture_manager__needs_flush(texture_manager_t* self);
void texture_manager__flush_to_gpu(texture_manager_t* self, SDL_GPUCommandBuffer* cmd_buffer, SDL_GPUCopyPass* opt_copy_pass);

bool texture_manager__is_valid(texture_manager_t const* self, texture_id_t id);
bool texture_manager__get_texture_info(
    texture_manager_t const* self,
    texture_id_t id,
    uint32_t* out_width,
    uint32_t* out_height
);

texture_id_t texture_manager__create_texture(texture_manager_t* self);
void texture_manager__delete_texture(texture_manager_t* self, texture_id_t id);

bool texture_manager__upload_texture_data(
    texture_manager_t* self,
    texture_id_t id,
    void const* data,
    size_t width, size_t height
);

bool texture_manager__bind_texture(
    texture_manager_t* self,
    uint32_t slot,
    texture_id_t id,
    sampler_config_t sampler_config,
    SDL_GPURenderPass* render_pass
);