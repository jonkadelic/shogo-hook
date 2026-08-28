#pragma once

#include <stddef.h>
#include <stdint.h>

#include <lithtech/lithtech.h>

#include "mesh/mesh.h"
#include "mesh/tessellator.h"

typedef struct blitter {
    SDL_GPUDevice* device;
    mesh_t mesh;
} blitter_t;

bool blitter__init(blitter_t* self, SDL_GPUDevice* device, tessellator_t* tessellator);
void blitter__cleanup(blitter_t* self);

void blit__blit_to_screen(blitter_t* self, BlitRequest_t const* request, SDL_GPURenderPass* render_pass);
