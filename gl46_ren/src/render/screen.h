#pragma once

#include <hmm/hmm.h>

#include "render/mesh.h"
#include "render/texture.h"
#include "render/pixel_buffer.h"

typedef struct screen {
    bool locked;
    mesh_t mesh;
    pixel_buffer_t buffer;
} screen_t;

bool screen__init(screen_t* self, size_t width, size_t height);
void screen__cleanup(screen_t* self);

void* screen__lock(screen_t* self);
void screen__draw(screen_t* self);
void screen__clear(screen_t* self);