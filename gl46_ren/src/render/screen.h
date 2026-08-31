#pragma once

#include "render/mesh.h"
#include "render/texture.h"
#include "util/rect_buffer.h"

typedef struct screen {
    mesh_t mesh;
    texture_t texture;
    rect_buffer_t buffer;
} screen_t;

bool screen__init(screen_t* self, size_t width, size_t height);
void screen__cleanup(screen_t* self);

