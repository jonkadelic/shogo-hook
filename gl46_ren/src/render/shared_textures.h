#pragma once

#include <lithtech/lithtech.h>

#include "render/texture.h"

typedef struct shared_texture {
    uint64_t path_hash;
    texture_t texture;
} shared_texture_t;

typedef struct shared_texture_manager {
    size_t textures_len, textures_capacity;
    shared_texture_t* textures;
} shared_texture_manager_t;

bool shared_texture_manager__init(shared_texture_manager_t* self);
void shared_texture_manager__cleanup(shared_texture_manager_t* self);

texture_t* shared_texture_manager__get_texture(shared_texture_manager_t* self, SharedTexture_t* lt_texture);
