#include "./model_renderer.h"

#include "logger.h"
#include "renderer.h"
#include "util/util.h"

static bool update_node_matrices(GLuint ssbo, ModelInstance_t const* model_instance);
static bool node_matrix_recurse(
    ModelAnimNode_t const* node_from,
    size_t key_from,
    ModelAnimNode_t const* node_to,
    size_t key_to,
    float lerp,
    HMM_Mat4 const* parent_matrix,
    HMM_Mat4* matrices
);

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

    update_node_matrices(self->gl_node_matrix_ssbo, model_instance);

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

static bool update_node_matrices(GLuint ssbo, ModelInstance_t const* model_instance) {
    auto model_data = model_instance->m_Tracker.m_pModelData;
    auto anim_from = model_instance->m_Tracker.m_Frames[0].m_pAnimation;
    auto anim_to = model_instance->m_Tracker.m_Frames[1].m_pAnimation;
    auto lerp = model_instance->m_Tracker.m_fLerp;
    
    SDL_assert(model_data->m_nNodeMatrices == model_data->m_nNodes);

    static_assert(sizeof(HMM_Mat4) == sizeof(float[16]));
    HMM_Mat4* matrix_data = SDL_malloc(sizeof(HMM_Mat4) * model_data->m_nNodeMatrices);
    if (matrix_data == nullptr) {
        LOG_ERROR("Failed to alloc %zu matrices", model_data->m_nNodeMatrices);
        return false;
    }
    for (size_t i = 0; i < model_data->m_nNodeMatrices; i++) {
        matrix_data[i] = HMM_M4D(1.0f);
    }
    
    HMM_Mat4 identity = HMM_M4D(1.0f);

    // Interpolate base animation root position
    if (anim_from->m_pNodeTransforms != nullptr && anim_to->m_pNodeTransforms != nullptr) {
        uint32_t key_from = model_instance->m_Tracker.m_Frames[0].m_FrameIndex;
        uint32_t key_to = model_instance->m_Tracker.m_Frames[1].m_FrameIndex;
        
        HMM_Vec3 pos_from = *(HMM_Vec3*) &anim_from->m_pNodeTransforms[key_from].m_Position;
        HMM_Vec3 pos_to = *(HMM_Vec3*) &anim_to->m_pNodeTransforms[key_to].m_Position;
        
        // Interpolate and negate Z for handedness
        HMM_Vec3 pos_interp = HMM_LerpV3(pos_from, lerp, pos_to);
        pos_interp.Z = -pos_interp.Z;
        
        identity = HMM_Translate(pos_interp);
    }

    for (size_t i = 0; i < anim_from->unk_40->m_nChildNodes; i++) {
        auto anim_node_from = &anim_from->m_pRootNodes[i];
        auto anim_node_to = &anim_to->m_pRootNodes[i];

        node_matrix_recurse(
            anim_node_from,
            model_instance->m_Tracker.m_Frames[0].m_FrameIndex,
            anim_node_to,
            model_instance->m_Tracker.m_Frames[1].m_FrameIndex,
            lerp,
            &identity,
            matrix_data
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

static bool node_matrix_recurse(
    ModelAnimNode_t const* node_from,
    size_t key_from,
    ModelAnimNode_t const* node_to,
    size_t key_to,
    float lerp,
    HMM_Mat4 const* parent_matrix,
    HMM_Mat4* matrices
) {
    auto model_node = node_from->m_pNode;

    auto pos_lerp = HMM_LerpV3(
        *(HMM_Vec3*) &node_from->m_pNodeTransforms[key_from].m_Position,
        lerp,
        *(HMM_Vec3*) &node_to->m_pNodeTransforms[key_to].m_Position
    );
    pos_lerp.Z = -pos_lerp.Z;

    HMM_Quat quat_from = *(HMM_Quat*) &node_from->m_pNodeTransforms[key_from].m_Rotation;
    HMM_Quat quat_to = *(HMM_Quat*) &node_to->m_pNodeTransforms[key_to].m_Rotation;
    
    // Shortest path: check dot product and negate if needed
    float dot = quat_from.X * quat_to.X + quat_from.Y * quat_to.Y + 
                quat_from.Z * quat_to.Z + quat_from.W * quat_to.W;
    if (dot < 0.0f) {
        quat_to.X = -quat_to.X;
        quat_to.Y = -quat_to.Y;
        quat_to.Z = -quat_to.Z;
        quat_to.W = -quat_to.W;
    }
    
    // SLERP (Spherical Linear Interpolation)
    HMM_Quat rot_lerp = HMM_SLerp(quat_from, lerp, quat_to);
    rot_lerp.Z = -rot_lerp.Z;

    matrices[model_node->m_Index] = HMM_Mul(
        HMM_Translate(pos_lerp),
        HMM_QToM4(rot_lerp)
    );
    matrices[model_node->m_Index] = HMM_Mul(
        *parent_matrix,
        matrices[model_node->m_Index]
    );

    if (node_from->m_pChildren == nullptr) {
        return true;
    }

    for (size_t i = 0; i < model_node->m_nChildNodes; i++) {
        if (!node_matrix_recurse(
            &node_from->m_pChildren[i],
            key_from,
            &node_to->m_pChildren[i],
            key_to,
            lerp,
            &matrices[model_node->m_Index],
            matrices
        )) {
            return false;
        }
    }

    return true;
}