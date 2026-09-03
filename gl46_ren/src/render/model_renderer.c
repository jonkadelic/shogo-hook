#include "./model_renderer.h"

#include "logger.h"
#include "renderer.h"
#include "util/util.h"

bool model_renderer__init(model_renderer_t* self, ModelInstance_t const* model_instance) {
    OBJECT_ZERO_INIT(self);

    texture_t* texture = nullptr;
    auto textures = renderer__get_shared_textures();
    if (model_instance->m_pSkin == nullptr) {
        if (model_instance->unk_130 == nullptr) {
            if (model_instance->base.m_pClass != nullptr &&
                model_instance->base.m_pClass->m_ppTextureFilename != nullptr &&
                (*model_instance->base.m_pClass->m_ppTextureFilename) != nullptr
            ) {
                texture = shared_texture_manager__get_texture_by_filename(textures, (*model_instance->base.m_pClass->m_ppTextureFilename)->m_Chars);
            }
        } else {
            texture = shared_texture_manager__get_texture(textures, model_instance->unk_130);
        }
    } else {
        texture = shared_texture_manager__get_texture(textures, model_instance->m_pSkin);
    }

    if (texture == nullptr) {
        LOG_ERROR("Tried to init model renderer with unloaded skin");
        return false;
    }

    self->texture = texture;

    self->num_meshes = 1;
    self->meshes = SDL_malloc(sizeof(mesh_t) * self->num_meshes);
    if (self->meshes == nullptr) {
        LOG_ERROR("Failed to alloc %zu meshes", self->num_meshes);
        goto err;
    }

    auto t = renderer__get_tessellator();
    vertex_t vertices[3];
    index_t indices[3];

    // temp
    if (!mesh__init(&self->meshes[0])) {
        LOG_ERROR("Failed to init mesh");
        goto err;
    }

    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_FRONT);

    static size_t const WIND_ARRAY[3] = { 0, 2, 1 };
    for (size_t i = 0; i < model_instance->m_pModelData->m_nFaces; i++) {
        auto face = &model_instance->m_pModelData->m_pFaces[i];
        float const* face_uvs = &model_instance->m_pModelData->m_pUVs[i * 6];
        
        for (size_t j = 0; j < 3; j++) {
            auto vertex = &model_instance->m_pModelData->m_pVertices[face->m_Vertices[WIND_ARRAY[j]]];

            vertices[j].position[0] = vertex->m_Position.x;
            vertices[j].position[1] = vertex->m_Position.y;
            vertices[j].position[2] = -vertex->m_Position.z;

            vertices[j].color[0] = 1.0f;
            vertices[j].color[1] = 1.0f;
            vertices[j].color[2] = 1.0f;
            vertices[j].color[3] = 1.0f;

            vertices[j].uv[0] = face_uvs[WIND_ARRAY[j] * 2 + 0];
            vertices[j].uv[1] = face_uvs[WIND_ARRAY[j] * 2 + 1];

            indices[j] = t->indices_len + j;
        }

        tessellator__append_vertices(t, 3, vertices);
        tessellator__append_indices(t, 3, indices);
    }

    tessellator__upload_and_reset(t, &self->meshes[0]);

    return true;

err:
    model_renderer__cleanup(self);
    return false;
}

void model_renderer__cleanup(model_renderer_t* self) {
    for (size_t i = 0; i < self->num_meshes; i++) {
        mesh__cleanup(&self->meshes[i]);
    }

    SDL_free(self->meshes);
}

void model_renderer__draw(model_renderer_t* self, ModelInstance_t const* model_instance) {
    auto object = &model_instance->base;

    HMM_Mat4 projection_matrix = renderer__get_view_projection_matrix();
    HMM_Mat4 model_matrix = HMM_Translate(HMM_V3(object->m_Pos.x, object->m_Pos.y, object->m_Pos.z));
    model_matrix = HMM_MulM4(
        model_matrix,
        HMM_QToM4(HMM_Q(object->m_Rotation.m_Vec.x, object->m_Rotation.m_Vec.y, object->m_Rotation.m_Vec.z, object->m_Rotation.m_Spin))
    );
    model_matrix = HMM_MulM4(
        model_matrix,
        HMM_Scale(HMM_V3(object->m_Scale.x, object->m_Scale.y, object->m_Scale.z))
    );

    shader_t const* shader = &renderer__get_shaders()[SHADER_ID__WORLD];

    shader__bind(shader);

    shader__set_uniform_mat4f(shader, "u_projection", &projection_matrix);
    shader__set_uniform_mat4f(shader, "u_model", &model_matrix);
    shader__set_uniform_texture(shader, "u_texture", self->texture);

    glEnable(GL_DEPTH_TEST);

    // Draw meshes
    for (size_t i = 0; i < self->num_meshes; i++) {
        // Draw mesh
        mesh__draw(&self->meshes[i]);
    }

    glDisable(GL_DEPTH_TEST);
}