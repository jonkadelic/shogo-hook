#pragma once

#include <lithtech/lithtech.h>
#include <hmm/hmm.h>

#include "render/mesh.h"
#include "render/shader.h"
#include "render/surfaces.h"
#include "render/tessellator.h"

typedef struct blitter {
    surface_manager_t* surfaces;
    shader_t const* shader;
    mesh_t mesh;
    HMM_Mat4 proj_matrix;
} blitter_t;

bool blitter__init(blitter_t* self, size_t width, size_t height, surface_manager_t* surfaces, shader_t const* shader, tessellator_t* tessellator);
void blitter__cleanup(blitter_t* self);

void blitter__blit_request(blitter_t* self, BlitRequest_t const* request);
