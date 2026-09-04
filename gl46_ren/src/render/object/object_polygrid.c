#include "./object_polygrid.h"

#include "logger.h"
#include "render/object/objects.h"
#include "renderer.h"
#include "shaders/shaders.h"

void object_polygrid__draw(object_data_t* self, SceneDesc_t const* scene_desc, DObject_t const* object) {
    auto polygrid = (PolyGrid_t const*) object;
    auto data = &self->as_polygrid;

    auto shader = &renderer__get_shaders()[SHADER_ID__POLYGRID];

    // If mesh doesn't exist, init mesh
    if (!data->mesh_init) {
        if (!mesh__init(&data->mesh, sizeof(vertex_t))) {
            LOG_ERROR("Failed to init polygrid mesh");
            return;
        }

        data->mesh_init = true;
    }

    // If size doesn't match, re-mesh
    auto tessellator = renderer__get_tessellator();
    if (data->width != polygrid->m_Width || data->height != polygrid->m_Height) {
        // Append all vertices
        for (uint32_t y = 0; y < polygrid->m_Height; y++) {
            for (uint32_t x = 0; x < polygrid->m_Width; x++) {
                tessellator__append_vertex(
                    tessellator,
                    &(vertex_t) {
                        .position = { (float) x, 0.0f, (float) y },
                        .color = { 1.0f, 1.0f, 1.0f, 1.0f },
                        // incorrect, but good enough for now
                        .uv = { (float) x / (float) polygrid->m_Width, (float) y / (float) polygrid->m_Height }
                    } 
                );
            }
        }

        // Append all indices - separate loop because square count is 1 smaller on both axes
        for (uint32_t y = 0; y < polygrid->m_Height - 1; y++) {
            for (uint32_t x = 0; x < polygrid->m_Width - 1; x++) {
                size_t sv[4] = {
                    (y * polygrid->m_Width) + x, // 0, 0
                    (y * polygrid->m_Width) + (x + 1), // 1, 0
                    ((y + 1) * polygrid->m_Width) + x, // 0, 1
                    ((y + 1) * polygrid->m_Width) + (x + 1), // 1, 1
                };
                
                tessellator__append_indices(
                    tessellator,
                    6,
                    (index_t[6]) {
                        sv[0], sv[2], sv[1],
                        sv[1], sv[2], sv[3]
                    }
                );
            }
        }

        // Upload new mesh
        tessellator__upload_and_reset(tessellator, &data->mesh);

        // Update size
        data->width = polygrid->m_Width;
        data->height = polygrid->m_Height;
    }

    // Create SSBOs if needed
    if (data->gl_offsets_ssbo == 0) {
        glCreateBuffers(1, &data->gl_offsets_ssbo);
        if (data->gl_offsets_ssbo == 0) {
            LOG_ERROR("Failed to init offsets SSBO");
            return;
        }
    }
    if (data->gl_colors_ssbo == 0) {
        glCreateBuffers(1, &data->gl_colors_ssbo);
        if (data->gl_colors_ssbo == 0) {
            LOG_ERROR("Failed to init colors SSBO");
            return;
        }
    }

    // Upload new offsets to SSBO
    glNamedBufferData(data->gl_offsets_ssbo, sizeof(int8_t) * polygrid->m_Width * polygrid->m_Height, polygrid->m_pData, GL_STREAM_DRAW);

    // Upload new colors to SSBO
    glNamedBufferData(data->gl_colors_ssbo, sizeof(float) * 256 * 4, polygrid->m_ColorTable, GL_STREAM_DRAW);

    HMM_Mat4 projection_matrix = renderer__get_view_projection_matrix();

    // translation * rotation * scale
    HMM_Mat4 model_matrix = HMM_Translate(HMM_V3(object->m_Pos.x, object->m_Pos.y, object->m_Pos.z));
    model_matrix = HMM_MulM4(
        model_matrix,
        HMM_QToM4(HMM_Q(object->m_Rotation.m_Vec.x, object->m_Rotation.m_Vec.y, object->m_Rotation.m_Vec.z, object->m_Rotation.m_Spin))
    );
    model_matrix = HMM_MulM4(
        model_matrix,
        HMM_Scale(HMM_V3(object->m_Scale.x, object->m_Scale.y, object->m_Scale.z))
    );
    model_matrix = HMM_MulM4(
        model_matrix,
        HMM_Translate(HMM_V3((polygrid->m_Width - 1) * -0.5f, 0.0f, (polygrid->m_Height - 1) * -0.5f))
    );

    // Bind shader
    shader__bind(shader);

    // Set matrices on shader
    shader__set_uniform_mat4f(shader, "u_projection", &projection_matrix);
    shader__set_uniform_mat4f(shader, "u_model", &model_matrix);

    // Bind texture
    texture_t* texture = shared_texture_manager__get_texture(renderer__get_shared_textures(), polygrid->m_SpriteTracker.m_pCurFrame->m_pTex);
    if (texture == nullptr) {
        LOG_WARNING("Failed to obtain shared texture for polygrid");
        return;
    }

    shader__set_uniform_texture(shader, "u_texture", texture);

    // Bind SSBOs
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, data->gl_offsets_ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, data->gl_colors_ssbo);

    glEnable(GL_DEPTH_TEST);

    // Draw mesh
    mesh__draw(&data->mesh);

    glDisable(GL_DEPTH_TEST);
}

void object_polygrid__cleanup(object_data_t* self) {
    object_polygrid_t* data = &self->as_polygrid;

    mesh__cleanup(&data->mesh);
    data->mesh_init = false;

    glDeleteBuffers(1, &data->gl_offsets_ssbo);
    data->gl_offsets_ssbo = 0;

    data->width = 0;
    data->height = 0;
}
