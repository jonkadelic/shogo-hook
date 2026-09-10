#include "./object_model.h"

#include "logger.h"
#include "render/models.h"
#include "render/object/objects.h"
#include "renderer.h"

void object_model__draw(object_data_t* self, SceneDesc_t const* scene_desc, DObject_t const* object) {
    auto model = (ModelInstance_t const*) object;
    auto data = &self->as_model;

    // If model doesn't exist or data doesn't match, init it
    if (data->renderer == nullptr || data->last_model != model->m_Tracker.m_pModelData) {
        data->renderer = model_manager__get_renderer(
            renderer__get_models(),
            model->m_Tracker.m_pModelData
        );

        if (data->renderer == nullptr) {
            return;
        }
    }

    model_renderer__draw(data->renderer, model);
}

void object_model__cleanup(object_data_t* self) {
    auto data = &self->as_model;

    data->renderer = nullptr;
}
