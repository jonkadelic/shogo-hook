#include "./world_renderer.h"

#include <hmm/hmm.h>

#include "logger.h"
#include "util/util.h"

static bool rebuild_main_world(world_renderer_t* self, MainWorld_t const* main_world);
static bool rebuild_world_models(world_renderer_t* self, MainWorld_t const* main_world);

bool world_renderer__init(
    world_renderer_t* self,
    MainWorld_t const* main_world
) {
    OBJECT_ZERO_INIT(self);

    // Sanity checks
    if (main_world == nullptr || main_world->m_pWorldBsp == nullptr) {
        LOG_ERROR("Failed to init world renderer; bad input");
        goto err;
    }

    if (!rebuild_main_world(self, main_world)) {
        LOG_ERROR("Failed to build main world");
        goto err;
    }
    self->main_world = main_world;

    return true;

err:
    world_renderer__cleanup(self);
    return false;
}

void world_renderer__cleanup(world_renderer_t* self) {
    world_bsp_renderer__cleanup(&self->main_model);
}

void world_renderer__draw(world_renderer_t* self, MainWorld_t const* main_world) {
    if (main_world != self->main_world) {
        rebuild_main_world(self, main_world);
        self->main_world = main_world;
    }

    world_bsp_renderer__draw(&self->main_model, HMM_M4D(1.0f), main_world->m_pWorldBsp);
}

static bool rebuild_main_world(world_renderer_t* self, MainWorld_t const* main_world) {
    world_bsp_renderer__cleanup(&self->main_model);
    if (!world_bsp_renderer__init(&self->main_model, main_world->m_pWorldBsp)) {
        LOG_ERROR("Failed to init main world model");
        return false;
    }

    return true;
}
