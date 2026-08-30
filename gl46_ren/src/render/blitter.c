#include "./blitter.h"

#include "util/util.h"
#include "logger.h"

bool blitter__init(blitter_t* self, size_t width, size_t height, surface_manager_t* surfaces, shader_t const* shader, tessellator_t* tessellator) {
    OBJECT_ZERO_INIT(self);

    self->surfaces = surfaces;
    self->shader = shader;

    if (!mesh__init(&self->mesh)) {
        LOG_ERROR("Failed to init mesh");
        goto err;
    }

    tessellator__append_vertices(tessellator,
        4,
        (vertex_t[4]) {
            (vertex_t) {
                .position = { 0.0f, 0.0f, 0.0f },
                .color = { 1.0f, 1.0f, 1.0f, 1.0f },
                .uv = { 0.0f, 0.0f },
            },
            (vertex_t) {
                .position = { 1.0f, 0.0f, 0.0f },
                .color = { 1.0f, 1.0f, 1.0f, 1.0f },
                .uv = { 1.0f, 0.0f },
            },
            (vertex_t) {
                .position = { 0.0f, 1.0f, 0.0f },
                .color = { 1.0f, 1.0f, 1.0f, 1.0f },
                .uv = { 0.0f, 1.0f },
            },
            (vertex_t) {
                .position = { 1.0f, 1.0f, 0.0f },
                .color = { 1.0f, 1.0f, 1.0f, 1.0f },
                .uv = { 1.0f, 1.0f },
            },
        }
    );
    tessellator__append_indices(tessellator,
        6,
        (index_t[6]) {
            0, 2, 1,
            1, 2, 3
        }
    );
    tessellator__upload_and_reset(tessellator, &self->mesh);

    self->proj_matrix = HMM_Orthographic_RH_NO(0.0f, (float) width, (float) height, 0.0f, -1.0f, 1.0f);

    return true;

err:
    blitter__cleanup(self);
    return false;
}

void blitter__cleanup(blitter_t* self) {
    mesh__cleanup(&self->mesh);
}

void blitter__blit_request(blitter_t* self, BlitRequest_t const* request) {
    if (request->m_BlitOptions == 0x04) {
        return;
    }

    surface_t* surface = request->m_pSurface;
    DRect_t const* dest = request->m_pDestRect;

    surface__update_texture(surface,
        (request->m_BlitOptions & BlitRequestOptions_Transparent) != 0,
        request->m_TransparentColor
    );

    if ((request->m_BlitOptions & BlitRequestOptions_Transparent) != 0) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    shader__bind(self->shader);
    shader__set_uniform_texture(self->shader, "u_texture", &surface->texture);

    auto model_matrix = HMM_MulM4(
        HMM_Translate((HMM_Vec3) { dest->left, dest->top, 0.0f }),
        HMM_Scale((HMM_Vec3) { dest->right - dest->left, dest->bottom - dest->top })
    );

    shader__set_uniform_mat4f(self->shader, "u_projection", &self->proj_matrix);
    shader__set_uniform_mat4f(self->shader, "u_model", &model_matrix);

    mesh__draw(&self->mesh);
}
