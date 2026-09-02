#include "./world.h"

#include <hmm/hmm.h>

#include "logger.h"
#include "renderer.h"
#include "util/util.h"

static bool build_world(world_t* self);

static bool build_world_model(
    world_t* self,
    size_t index
);

bool world__init(
    world_t* self,
    SceneDesc_t const* scene,
    tessellator_t* tessellator,
    shared_texture_manager_t* textures
) {
    OBJECT_ZERO_INIT(self);

    SharedTexture_t** shared_texture_cache = nullptr;

    if (scene->m_hRenderContext == nullptr || scene->m_hRenderContext->m_pMainWorld == nullptr) {
        LOG_ERROR("Failed to init world renderer; bad input");
        goto err;
    }
    auto main_world = scene->m_hRenderContext->m_pMainWorld;

    // Sanity checks
    if (main_world == nullptr || main_world->m_pWorldBsp == nullptr || main_world->m_pWorldBsp->m_nTextureNames == 0) {
        LOG_ERROR("Failed to init world renderer; bad input");
        goto err;
    }

    self->world = main_world;
    self->tessellator = tessellator;
    self->textures = textures;

    if (!build_world(self)) {
        LOG_ERROR("Failed to build world");
        goto err;
    }

    return true;

err:
    world__cleanup(self);
    return false;
}

void world__cleanup(world_t* self) {
    for (size_t i = 0; i < self->models_len; i++) {
        mesh__cleanup(&self->models[i].mesh);
    }

    SDL_free(self->models);
    self->models = nullptr;
    self->models_len = 0;
}

void world__draw(world_t* self, SceneDesc_t const* scene) {
    if (scene->m_hRenderContext != nullptr && scene->m_hRenderContext->m_pMainWorld != self->world) {
        for (size_t i = 0; i < self->models_len; i++) {
            mesh__cleanup(&self->models[i].mesh);
        }

        SDL_free(self->models);

        self->world = scene->m_hRenderContext->m_pMainWorld;
        if (!build_world(self)) {
            LOG_ERROR("Failed to build world");
            return;
        }
    }

    HMM_Mat4 projection_matrix = renderer__get_view_projection_matrix();
    HMM_Mat4 model_matrix = HMM_M4D(1.0f);

    shader_t const* shader = &renderer__get_shaders()[SHADER_ID__WORLD];
    
    shader__bind(shader);

    shader__set_uniform_mat4f(shader, "u_projection", &projection_matrix);
    shader__set_uniform_mat4f(shader, "u_model", &model_matrix);

    glEnable(GL_DEPTH_TEST);

    // Draw models
    for (size_t i = 0; i < self->models_len; i++) {
        // Bind texture
        shader__set_uniform_texture(shader, "u_texture", self->models[i].texture);

        // Draw mesh
        mesh__draw(&self->models[i].mesh);
    }

    glDisable(GL_DEPTH_TEST);
}

static bool build_world(world_t* self) {
    SharedTexture_t** shared_texture_cache = nullptr;
    auto main_world = self->world;

    // Init model storage
    self->models_len = main_world->m_pWorldBsp->m_nTextureNames;
    self->models = SDL_calloc(self->models_len, sizeof(world_model_t));
    if (self->models == nullptr) {
        LOG_ERROR("Failed to alloc %zu world models", self->models_len);
        goto err;
    }

    // Init texture cache
    shared_texture_cache = SDL_calloc(self->models_len, sizeof(shared_texture_t*));
    if (shared_texture_cache == nullptr) {
        LOG_ERROR("Failed to alloc shared texture cache (%zu entries)", self->models_len);
        goto err;
    }
    for (size_t i = 0; i < main_world->m_pWorldBsp->m_nSurfaces; i++) {
        auto surface = &main_world->m_pWorldBsp->m_Surfaces[i];
        if (surface->m_iTexture >= self->models_len) {
            LOG_ERROR("Weird surface texture");
            goto err;
        }

        shared_texture_cache[surface->m_iTexture] = surface->m_pTexture;
    }

    // Set up models
    for (size_t i = 0; i < self->models_len; i++) {
        auto world_model = &self->models[i];
        if (!mesh__init(&world_model->mesh)) {
            LOG_ERROR("Failed to init world model mesh");
            goto err;
        }
        world_model->texture = shared_texture_manager__get_texture(self->textures, shared_texture_cache[i]);
        if (world_model->texture == nullptr) {
            LOG_ERROR("Failed to get texture for world model mesh");
            goto err;
        }

        if (!build_world_model(self, i)) {
            LOG_ERROR("Failed to set up world model %zu", i);
            goto err;
        }
    }

    SDL_free(shared_texture_cache);
    shared_texture_cache = nullptr;

    LOG_INFO("Loaded world with %zu models", self->models_len);

    return true;

err:
    SDL_free(shared_texture_cache);
    return false;
}

static bool build_world_model(
    world_t* self,
    size_t index
) {
    auto bsp = self->world->m_pWorldBsp;
    auto tessellator = self->tessellator;
    auto world_model = &self->models[index];

    size_t num_vertices = 0;
    vertex_t* vertices = nullptr;
    size_t num_indices = 0;
    index_t* indices = nullptr;

    for (size_t i = 0; i < bsp->m_nPolies; i++) {
        auto poly = bsp->m_Polies[i];
        if (poly->m_pSurface->m_iTexture != index) {
            continue;
        }
        if ((poly->m_pSurface->m_SurfaceFlags & (SurfaceFlags_NonExistent | SurfaceFlags_Invisible | SurfaceFlags_Sky)) > 0) {
            continue;
        }

        if (poly->m_nVertices < 3) {
            LOG_WARNING("Weird polygon with < 3 vertices");
            continue;
        }

        size_t vertices_base = num_vertices;
        size_t indices_base = num_indices;

        // Resize vertices
        size_t new_num_vertices = num_vertices + poly->m_nVertices;
        vertex_t* new_vertices = SDL_realloc(vertices, new_num_vertices * sizeof(vertex_t));
        if (new_vertices == nullptr) {
            LOG_ERROR("Failed to alloc %zu vertices", new_num_vertices);
            continue;
        }

        // Resize indices
        size_t new_num_indices = num_indices + (size_t)(poly->m_nVertices - 2) * 3;
        index_t* new_indices = SDL_realloc(indices, new_num_indices * sizeof(index_t));
        if (new_indices == nullptr) {
            SDL_free(new_vertices);
            LOG_ERROR("Failed to alloc %zu indices", new_num_indices);
            continue;
        }

        num_vertices = new_num_vertices;
        vertices = new_vertices;
        num_indices = new_num_indices;
        indices = new_indices;

        // Instantiate vertices
        for (size_t j = 0; j < poly->m_nVertices; j++) {
            auto vertex = &poly->m_Vertices[j];
            auto out_vertex = &vertices[vertices_base + j];
            
            out_vertex->position[0] = vertex->m_pVertexData->x;
            out_vertex->position[1] = vertex->m_pVertexData->y;
            out_vertex->position[2] = vertex->m_pVertexData->z;

            out_vertex->color[0] = vertex->m_Color[2] / 255.0f; // r
            out_vertex->color[1] = vertex->m_Color[1] / 255.0f; // g
            out_vertex->color[2] = vertex->m_Color[0] / 255.0f; // b
            out_vertex->color[3] = vertex->m_Color[3] / 255.0f; // a

            out_vertex->uv[0] = vertex->m_UV[0] / world_model->texture->width;
            out_vertex->uv[1] = vertex->m_UV[1] / world_model->texture->height;
        }

        // Instantiate indices
        for (size_t j = 0; j < poly->m_nVertices - 2; j++) {
            indices[indices_base + (j * 3) + 0] = vertices_base + 0;
            indices[indices_base + (j * 3) + 1] = vertices_base + j + 1;
            indices[indices_base + (j * 3) + 2] = vertices_base + j + 2;
        }
    }

    tessellator__append_vertices(tessellator, num_vertices, vertices);
    tessellator__append_indices(tessellator, num_indices, indices);
    tessellator__upload_and_reset(tessellator, &world_model->mesh);

    SDL_free(vertices);
    SDL_free(indices);

    return true;
}