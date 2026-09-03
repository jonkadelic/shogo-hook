#pragma once

#include <stdint.h>

#include <lithtech/lithtech.h>

#include "render/world/world_bsp_renderer.h"

typedef struct world_renderer {
    MainWorld_t const* main_world;
    world_bsp_renderer_t main_model;
} world_renderer_t;

bool world_renderer__init(
    world_renderer_t* self,
    MainWorld_t const* main_world
);
void world_renderer__cleanup(world_renderer_t* self);

void world_renderer__draw(world_renderer_t* self, MainWorld_t const* main_world);
