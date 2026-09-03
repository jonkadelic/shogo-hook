#include "./object_worldmodel.h"

#include "logger.h"
#include "render/object/objects.h"

void object_worldmodel__draw(object_data_t* self, SceneDesc_t const* scene_desc, DObject_t const* object) {
    auto worldmodel = (WorldModelInstance_t*) object;
    auto data = &self->as_worldmodel;

    if (scene_desc->m_ModelHookFn != nullptr) {
        ModelHookData_t hook_data = {
            .m_hObject = object,
            .m_ObjectFlags = object->m_Flags,
            .m_Flags = 1,
        };
        scene_desc->m_ModelHookFn(&hook_data, scene_desc->m_ModelHookUser);
    }

    if (!data->renderer_init) {
        if (!world_bsp_renderer__init(&data->renderer, worldmodel->m_pWorldData->m_pValidBsp)) {
            LOG_ERROR("Failed to init worldmodel object renderer");
            return;
        }

        data->renderer_init = true;
    }

    HMM_Mat4 model_matrix;
    for (size_t row = 0; row < 4; row++) {
        for (size_t column = 0; column < 4; column++) {
            model_matrix.Elements[column][row] =
                worldmodel->m_Transform.m[row][column];
        }
    }
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    world_bsp_renderer__draw(&data->renderer, model_matrix, worldmodel->m_pWorldData->m_pValidBsp);
    glDisable(GL_BLEND);
}

void object_worldmodel__cleanup(object_data_t* self) {
    auto data = &self->as_worldmodel;

    world_bsp_renderer__cleanup(&data->renderer);
    data->renderer_init = false;
}
