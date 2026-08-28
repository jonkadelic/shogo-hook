#pragma once

#include <stdint.h>

#include <SDL3/SDL.h>

#include <lithtech/lithtech.h>

#include "mesh/tessellator.h"
#include "render/blitter.h"
#include "render/surface.h"
#include "shader/shader.h"

typedef struct renderer {
    bool initialized;

    SDL_Window* window;
    SDL_GPUDevice* device;

    bool in_3d;
    bool in_2d;

    tessellator_t tessellator;
    surface_manager_t surfaces;
    blitter_t blitter;

    shader_t shaders[NUM_SHADER_IDS];

    bool should_clear;
    float clear_color[4];

    SDL_GPUCommandBuffer* cmd_buffer;
    SDL_GPURenderPass* render_pass;
} renderer_t;

renderer_t* renderer__get(void);

bool renderer__init(renderer_t* self, void* hwnd, RMode_t const* mode);
void renderer__cleanup(renderer_t* self);

void renderer__clear(renderer_t* self, float r, float g, float b, float a);

void renderer__start_draw(renderer_t* self);
void renderer__end_draw(renderer_t* self);

void renderer__draw_scene(renderer_t* self, SceneDesc_t const* scene_desc);
void renderer__blit_to_screen(renderer_t* self, BlitRequest_t const* request);
