#pragma once

#include <stdint.h>

#include <lithtech/lithtech.h>

#include "render/world/world_bsp_renderer.h"

typedef struct world_renderer_model {
    mesh_t mesh;
    texture_t* texture;
} world_renderer_model_t;

typedef struct world_renderer {
    MainWorld_t const* main_world;

    world_bsp_renderer_t main_model;
    size_t models_len;
    world_bsp_renderer_t* models;
} world_renderer_t;

bool world_renderer__init(
    world_renderer_t* self,
    MainWorld_t const* main_world
);
void world_renderer__cleanup(world_renderer_t* self);

void world_renderer__draw(world_renderer_t* self, MainWorld_t const* main_world);
