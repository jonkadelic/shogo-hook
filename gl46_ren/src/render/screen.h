#pragma once

#include <hmm/hmm.h>

#include "render/mesh.h"
#include "render/texture.h"
#include "util/rect_buffer.h"

typedef struct screen {
    bool locked;
    mesh_t mesh;
    texture_t texture;
    rect_buffer_t buffer;
    rect_buffer_t buffer_32;
    HMM_Mat4 proj_matrix;
} screen_t;

bool screen__init(screen_t* self, size_t width, size_t height);
void screen__cleanup(screen_t* self);

void* screen__lock(screen_t* self);
void screen__draw(screen_t* self);
void screen__clear(screen_t* self);
