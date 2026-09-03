#include "./polygrid.h"

#include "logger.h"
#include "objects.h"
#include "renderer.h"
#include "shaders/shaders.h"

void polygrid__draw(object_data_t* self, tessellator_t* tessellator, DObject_t const* object) {
    auto polygrid = (PolyGrid_t const*) object;
    auto polygrid_data = &self->as_polygrid;

    auto shader = &renderer__get_shaders()[SHADER_ID__POLYGRID];

    // If mesh doesn't exist, init mesh
    if (!polygrid_data->mesh_init) {
        if (!mesh__init(&polygrid_data->mesh)) {
            LOG_ERROR("Failed to init polygrid mesh");
            return;
        }

        polygrid_data->mesh_init = true;
    }

    // If size doesn't match, re-mesh
    if (polygrid_data->width != polygrid->m_Width || polygrid_data->height != polygrid->m_Height) {
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
        tessellator__upload_and_reset(tessellator, &polygrid_data->mesh);

        // Update size
        polygrid_data->width = polygrid->m_Width;
        polygrid_data->height = polygrid->m_Height;
    }

    // Create SSBOs if needed
    if (polygrid_data->gl_offsets_ssbo == 0) {
        glCreateBuffers(1, &polygrid_data->gl_offsets_ssbo);
        if (polygrid_data->gl_offsets_ssbo == 0) {
            LOG_ERROR("Failed to init offsets SSBO");
            return;
        }
    }
    if (polygrid_data->gl_colors_ssbo == 0) {
        glCreateBuffers(1, &polygrid_data->gl_colors_ssbo);
        if (polygrid_data->gl_colors_ssbo == 0) {
            LOG_ERROR("Failed to init colors SSBO");
            return;
        }
    }

    // Upload new offsets to SSBO
    glNamedBufferData(polygrid_data->gl_offsets_ssbo, sizeof(int8_t) * polygrid->m_Width * polygrid->m_Height, polygrid->m_pData, GL_STREAM_DRAW);

    // Upload new colors to SSBO
    glNamedBufferData(polygrid_data->gl_colors_ssbo, sizeof(float) * 256 * 4, polygrid->m_ColorTable, GL_STREAM_DRAW);

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
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, polygrid_data->gl_offsets_ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, polygrid_data->gl_colors_ssbo);

    // Draw mesh
    mesh__draw(&polygrid_data->mesh);
}

void polygrid__cleanup(object_data_t* self) {
    polygrid_t* polygrid = &self->as_polygrid;

    mesh__cleanup(&polygrid->mesh);
    polygrid->mesh_init = false;

    glDeleteBuffers(1, &polygrid->gl_offsets_ssbo);
    polygrid->gl_offsets_ssbo = 0;

    polygrid->width = 0;
    polygrid->height = 0;
}
