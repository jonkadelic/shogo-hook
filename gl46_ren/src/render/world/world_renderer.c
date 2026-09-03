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
    if (!rebuild_world_models(self, main_world)) {
        LOG_ERROR("Failed to build world models");
        goto err;
    }
    self->main_world = main_world;

    return true;

err:
    world_renderer__cleanup(self);
    return false;
}

void world_renderer__cleanup(world_renderer_t* self) {
    for (size_t i = 0; i < self->models_len; i++) {
        world_bsp_renderer__cleanup(&self->models[i]);
    }

    SDL_free(self->models);
    self->models = nullptr;
    self->models_len = 0;

    world_bsp_renderer__cleanup(&self->main_model);
}

void world_renderer__draw(world_renderer_t* self, MainWorld_t const* main_world) {
    if (main_world != self->main_world) {
        rebuild_main_world(self, main_world);
        rebuild_world_models(self, main_world);
        self->main_world = main_world;
    }

    if (main_world->m_nWorldModels != self->models_len) {
        rebuild_world_models(self, main_world);
    }

    world_bsp_renderer__draw(&self->main_model, main_world->m_pWorldBsp);

    for (size_t i = 0; i < self->models_len; i++) {
        world_bsp_renderer__draw(&self->models[i], main_world->m_pWorldModels[i]->m_pWorldBsp);
    }
}

static bool rebuild_main_world(world_renderer_t* self, MainWorld_t const* main_world) {
    world_bsp_renderer__cleanup(&self->main_model);
    if (!world_bsp_renderer__init(&self->main_model, main_world->m_pWorldBsp)) {
        LOG_ERROR("Failed to init main world model");
        return false;
    }

    return true;
}

static bool rebuild_world_models(world_renderer_t* self, MainWorld_t const* main_world) {
    for (size_t i = 0; i < self->models_len; i++) {
        world_bsp_renderer__cleanup(&self->models[i]);
    }
    SDL_free(self->models);
    self->models_len = 0;

    self->models = SDL_calloc(main_world->m_nWorldModels, sizeof(world_bsp_renderer_t));
    if (self->models == nullptr) {
        LOG_ERROR("Failed to alloc %zu world renderers", main_world->m_nWorldModels);
        return false;
    }
    self->models_len = main_world->m_nWorldModels;

    for (size_t i = 0; i < self->models_len; i++) {
        if (!world_bsp_renderer__init(&self->models[i], main_world->m_pWorldModels[i]->m_pValidBsp)) {
            LOG_ERROR("Failed to init world renderer %zu", i);
            return false;
        }
    }

    return true;
}
