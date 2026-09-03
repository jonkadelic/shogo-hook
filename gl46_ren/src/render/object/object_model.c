#include "./object_model.h"

#include "logger.h"
#include "render/object/objects.h"

void object_model__draw(object_data_t* self, SceneDesc_t const* scene_desc, DObject_t const* object) {
    auto model = (ModelInstance_t const*) object;
    auto data = &self->as_model;

    // If model doesn't exist, init it
    if (!data->renderer_init) {
        if (!model_renderer__init(&data->renderer, model)) {
            // LOG_ERROR("Failed to init model renderer");
            return;
        }

        data->renderer_init = true;
    }

    model_renderer__draw(&data->renderer, model);
}

void object_model__cleanup(object_data_t* self) {
    auto data = &self->as_model;

    model_renderer__cleanup(&data->renderer);
    data->renderer_init = false;
}
