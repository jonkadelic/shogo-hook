#include "./world_bsp_renderer.h"

#include <hmm/hmm.h>

#include "logger.h"
#include "render/shader.h"
#include "renderer.h"
#include "util/util.h"

static bool build_world_bsp_models(world_bsp_renderer_t* self);
static bool build_world_bsp_model(
    world_bsp_renderer_t* self,
    size_t index
);

bool world_bsp_renderer__init(
    world_bsp_renderer_t* self,
    WorldBsp_t const* world_bsp
) {
    OBJECT_ZERO_INIT(self);

    // Sanity checks
    if (world_bsp == nullptr || world_bsp->m_nTextureNames == 0) {
        LOG_ERROR("Failed to init world renderer; bad input");
        goto err;
    }

    self->world_bsp = world_bsp;

    if (!build_world_bsp_models(self)) {
        LOG_ERROR("Failed to build world");
        goto err;
    }

    return true;

err:
    world_bsp_renderer__cleanup(self);
    return false;
}

void world_bsp_renderer__cleanup(world_bsp_renderer_t* self) {
    for (size_t i = 0; i < self->models_len; i++) {
        mesh__cleanup(&self->models[i].mesh);
    }

    SDL_free(self->models);
    self->models = nullptr;
    self->models_len = 0;
}

void world_bsp_renderer__draw(world_bsp_renderer_t* self, WorldBsp_t const* world_bsp) {
    if (world_bsp != self->world_bsp) {
        for (size_t i = 0; i < self->models_len; i++) {
            mesh__cleanup(&self->models[i].mesh);
        }

        SDL_free(self->models);

        self->world_bsp = world_bsp;
        if (!build_world_bsp_models(self)) {
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
        if (self->models[i].texture == nullptr) continue;

        // Bind texture
        shader__set_uniform_texture(shader, "u_texture", self->models[i].texture);

        // Draw mesh
        mesh__draw(&self->models[i].mesh);
    }

    glDisable(GL_DEPTH_TEST);
}

static bool build_world_bsp_models(world_bsp_renderer_t* self) {
    texture_t** texture_cache = nullptr;

    // Init model storage
    self->models_len = self->world_bsp->m_nTextureNames;
    self->models = SDL_calloc(self->models_len, sizeof(world_bsp_model_t));
    if (self->models == nullptr) {
        LOG_ERROR("Failed to alloc %zu world models", self->models_len);
        goto err;
    }

    // Init texture cache
    auto textures = renderer__get_shared_textures();
    texture_cache = SDL_calloc(self->models_len, sizeof(texture_t*));
    if (texture_cache == nullptr) {
        LOG_ERROR("Failed to alloc shared texture cache (%zu entries)", self->models_len);
        goto err;
    }
    for (size_t i = 0; i < self->world_bsp->m_nSurfaces; i++) {
        auto surface = &self->world_bsp->m_Surfaces[i];
        if (surface->m_iTexture >= self->world_bsp->m_nTextureNames) {
            LOG_ERROR("Weird surface texture - aborting model build");
            goto err;
        }
        if (surface->m_pTexture != nullptr) {
            texture_cache[surface->m_iTexture] = shared_texture_manager__get_texture(textures, surface->m_pTexture);
        } else {
            texture_cache[surface->m_iTexture] = shared_texture_manager__get_texture_by_filename(
                textures,
                self->world_bsp->m_TextureNames[surface->m_iTexture]
            );
        }

        if (texture_cache[surface->m_iTexture] == nullptr) {
            LOG_WARNING("Surface missing texture");
            continue;
        }
    }

    // Set up models
    for (size_t i = 0; i < self->models_len; i++) {
        auto world_model = &self->models[i];
        if (!mesh__init(&world_model->mesh)) {
            LOG_ERROR("Failed to init world model mesh");
            goto err;
        }

        if (texture_cache[i] == nullptr) continue;

        world_model->texture = texture_cache[i];
        if (world_model->texture == nullptr) {
            LOG_ERROR("Failed to get texture for world model mesh");
            goto err;
        }

        if (!build_world_bsp_model(self, i)) {
            LOG_ERROR("Failed to set up world model %zu", i);
            goto err;
        }
    }

    SDL_free(texture_cache);
    texture_cache = nullptr;

    LOG_INFO("Loaded world with %zu models", self->models_len);

    return true;

err:
    SDL_free(texture_cache);
    return false;
}

static bool build_world_bsp_model(
    world_bsp_renderer_t* self,
    size_t index
) {
    auto tessellator = renderer__get_tessellator();
    auto world_model = &self->models[index];

    size_t num_vertices = 0;
    vertex_t* vertices = nullptr;
    size_t num_indices = 0;
    index_t* indices = nullptr;

    for (size_t i = 0; i < self->world_bsp->m_nPolies; i++) {
        auto poly = self->world_bsp->m_Polies[i];
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