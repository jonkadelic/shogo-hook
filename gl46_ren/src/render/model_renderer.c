#include "./model_renderer.h"

#include "logger.h"
#include "renderer.h"
#include "util/util.h"

static bool update_node_matrices(GLuint ssbo, ModelData_t const* model_data);

bool model_renderer__init(model_renderer_t* self, ModelData_t const* model_data) {
    OBJECT_ZERO_INIT(self);

    self->num_meshes = 1;
    self->meshes = SDL_malloc(sizeof(mesh_t) * self->num_meshes);
    if (self->meshes == nullptr) {
        LOG_ERROR("Failed to alloc %zu meshes", self->num_meshes);
        goto err;
    }

    auto t = renderer__get_model_tessellator();
    model_vertex_t vertices[3];
    index_t indices[3];

    // temp
    if (!mesh__init(&self->meshes[0], sizeof(model_vertex_t))) {
        LOG_ERROR("Failed to init mesh");
        goto err;
    }

    static size_t const WIND_ARRAY[3] = { 0, 2, 1 };
    for (size_t i = 0; i < model_data->m_nFaces; i++) {
        auto face = &model_data->m_pFaces[i];
        float const* face_uvs = &model_data->m_pUVs[i * 6];
        
        for (size_t j = 0; j < 3; j++) {
            auto vertex = &model_data->m_pVertices[face->m_Vertices[WIND_ARRAY[j]]];

            vertices[j].position[0] = vertex->m_Position.x;
            vertices[j].position[1] = vertex->m_Position.y;
            vertices[j].position[2] = -vertex->m_Position.z; // without this, model vertices are flipped

            vertices[j].color[0] = 1.0f;
            vertices[j].color[1] = 1.0f;
            vertices[j].color[2] = 1.0f;
            vertices[j].color[3] = 1.0f;

            vertices[j].uv[0] = face_uvs[WIND_ARRAY[j] * 2 + 0];
            vertices[j].uv[1] = face_uvs[WIND_ARRAY[j] * 2 + 1];

            vertices[j].node_idx = vertex->m_NodeIndex;

            indices[j] = t->indices_len + j;
        }

        tessellator__append_vertices(t, 3, vertices);
        tessellator__append_indices(t, 3, indices);
    }

    tessellator__upload_and_reset(t, &self->meshes[0]);

    glCreateBuffers(1, &self->gl_node_matrix_ssbo);
    if (self->gl_node_matrix_ssbo == 0) {
        LOG_ERROR("Failed to init node transform SSBO");
        goto err;
    }

    update_node_matrices(self->gl_node_matrix_ssbo, model_data);

    return true;

err:
    model_renderer__cleanup(self);
    return false;
}

void model_renderer__cleanup(model_renderer_t* self) {
    glDeleteBuffers(1, &self->gl_node_matrix_ssbo);

    for (size_t i = 0; i < self->num_meshes; i++) {
        mesh__cleanup(&self->meshes[i]);
    }

    SDL_free(self->meshes);
}

void model_renderer__draw(model_renderer_t* self, ModelInstance_t const* model_instance) {
    auto object = &model_instance->base;

    // update_node_matrices(self->gl_node_matrix_ssbo, model_instance->m_pModelData);

    if (model_instance->m_pSkin == nullptr) {
        return;
    }

    texture_t* texture = shared_texture_manager__get_texture(
        renderer__get_shared_textures(),
        model_instance->m_pSkin
    );

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

    shader_t const* shader = &renderer__get_shaders()[SHADER_ID__MODEL];

    shader__bind(shader);

    shader__set_uniform_mat4f(shader, "u_projection", &projection_matrix);
    shader__set_uniform_mat4f(shader, "u_model", &model_matrix);
    shader__set_uniform_texture(shader, "u_texture", texture);

    glEnable(GL_DEPTH_TEST);

    // Bind matrix SSBO
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, self->gl_node_matrix_ssbo);

    // Draw meshes
    for (size_t i = 0; i < self->num_meshes; i++) {
        // Draw mesh
        mesh__draw(&self->meshes[i]);
    }

    glDisable(GL_DEPTH_TEST);
}

static bool update_node_matrices(GLuint ssbo, ModelData_t const* model_data) {
    SDL_assert(model_data->m_nNodeMatrices == model_data->m_nNodes);

    static_assert(sizeof(HMM_Mat4) == sizeof(float[16]));
    HMM_Mat4* matrix_data = SDL_calloc(1, sizeof(HMM_Mat4) * model_data->m_nNodeMatrices);
    if (matrix_data == nullptr) {
        LOG_ERROR("Failed to alloc %zu matrices", model_data->m_nNodeMatrices);
        return false;
    }

    HMM_Mat4 handedness = HMM_Scale(HMM_V3(1.0f, 1.0f, -1.0f));

    for (size_t i = 0; i < model_data->m_nNodeMatrices; i++) {
        auto in_matrix = &model_data->m_pNodeMatrices[i];
        auto out_matrix = &matrix_data[i];

        static_assert(sizeof(*in_matrix) == sizeof(*out_matrix));
        *out_matrix = HMM_TransposeM4(*(HMM_Mat4*) in_matrix);
        *out_matrix = HMM_MulM4(
            HMM_MulM4(handedness, *out_matrix),
            handedness
        );
    }

    // We now have a buffer of matrix data. Let's upload it:
    glNamedBufferData(
        ssbo,
        sizeof(float[16]) * model_data->m_nNodeMatrices,
        matrix_data,
        GL_STREAM_DRAW
    );

    SDL_free(matrix_data);

    return true;
}
