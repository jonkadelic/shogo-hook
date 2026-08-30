#pragma once

#include <stdint.h>

#include <SDL3/SDL.h>

typedef struct sampler {
    uint64_t config_hash;
    SDL_GPUSampler* sampler;
} sampler_t;

typedef struct sampler_config {
    SDL_GPUFilter min_filter;
    SDL_GPUFilter mag_filter;
    SDL_GPUSamplerAddressMode wrap_s;
    SDL_GPUSamplerAddressMode wrap_t;
} sampler_config_t;

typedef struct sampler_manager {
    SDL_GPUDevice* device;
    size_t samplers_len, samplers_capacity;
    sampler_t* samplers;
} sampler_manager_t;

bool sampler_manager__init(sampler_manager_t* self, SDL_GPUDevice* device);
void sampler_manager__cleanup(sampler_manager_t* self);

SDL_GPUSampler* sampler_manager__get_sampler(sampler_manager_t* self, sampler_config_t config);

