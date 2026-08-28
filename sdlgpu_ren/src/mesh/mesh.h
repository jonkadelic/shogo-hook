#pragma once

#include <SDL3/SDL.h>

typedef struct vertex {
    float x, y, z;
    float r, g, b, a;
    float u, v;
} vertex_t;

typedef struct mesh {
    SDL_GPUDevice* device;
    size_t vertices_len, vertices_capacity;
    SDL_GPUBuffer* vertices;
    size_t indices_len, indices_capacity;
    SDL_GPUBuffer* indices;
} mesh_t;

bool mesh__init(mesh_t* self, SDL_GPUDevice* device);
void mesh__cleanup(mesh_t* self);

void mesh__draw(mesh_t* self, SDL_GPURenderPass* render_pass);
