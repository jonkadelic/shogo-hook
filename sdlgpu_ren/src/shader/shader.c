#include "./shader.h"

#include <stddef.h>

#include <SDL3/SDL.h>

#include "util/util.h"
#include "util/hash.h"

#define PIPELINE_ALLOC_SIZE (2)

// config_hash must be first member for `compare_pipelines_by_hash` to work
static_assert(offsetof(shader_pipeline_t, config_hash) == 0);

static int compare_pipelines_by_hash(void const* a, void const* b);

static shader_pipeline_t* create_pipeline(shader_t* self, shader_config_t const* config, uint64_t config_hash);

bool shader__init(
    shader_t* self,
    SDL_GPUDevice* device,
    size_t vertex_src_len, void const* vertex_src,
    size_t fragment_src_len, void const* fragment_src,
    size_t num_vertex_buffers, SDL_GPUVertexBufferDescription const* vertex_buffers,
    size_t num_vertex_attributes, SDL_GPUVertexAttribute const* vertex_attributes,
    size_t num_color_targets, SDL_GPUColorTargetDescription const* color_targets
) {
    ZERO_INIT_STRUCT(self);

    self->device = device;

    // Create vertex shader info
    SDL_GPUShaderCreateInfo vertex_shader_info = {
        .code_size = vertex_src_len,
        .code = vertex_src,
        .entrypoint = "main",
        .format = SDL_GPU_SHADERFORMAT_SPIRV,
        .stage = SDL_GPU_SHADERSTAGE_VERTEX,
    };

    // Create vertex shader
    self->vertex_shader = SDL_CreateGPUShader(device, &vertex_shader_info);
    if (self->vertex_shader == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_GPU, "Failed to create vertex shader: %s", SDL_GetError());
        goto err;
    }

    // Create fragment shader info
    SDL_GPUShaderCreateInfo fragment_shader_info = {
        .code_size = fragment_src_len,
        .code = fragment_src,
        .entrypoint = "main",
        .format = SDL_GPU_SHADERFORMAT_SPIRV,
        .stage = SDL_GPU_SHADERSTAGE_FRAGMENT,
    };

    // Create fragment shader
    self->fragment_shader = SDL_CreateGPUShader(device, &fragment_shader_info);
    if (self->fragment_shader == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_GPU, "Failed to create fragment shader: %s", SDL_GetError());
        goto err;
    }

    // Duplicate vertex buffers
    self->num_vertex_buffers = num_vertex_buffers;
    self->vertex_buffers = SDL_malloc(sizeof(SDL_GPUVertexBufferDescription) * num_vertex_buffers);
    if (self->vertex_buffers == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "Failed to allocate vertex buffer descriptions");
        goto err;
    }
    SDL_memcpy(self->vertex_buffers, vertex_buffers, sizeof(SDL_GPUVertexBufferDescription) * num_vertex_buffers);

    // Duplicate vertex attributes
    self->num_vertex_attributes = num_vertex_attributes;
    self->vertex_attributes = SDL_malloc(sizeof(SDL_GPUVertexAttribute) * num_vertex_attributes);
    if (self->vertex_attributes == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "Failed to allocate vertex attributes");
        goto err;
    }
    SDL_memcpy(self->vertex_attributes, vertex_attributes, sizeof(SDL_GPUVertexAttribute) * num_vertex_attributes);

    // Duplicate color targets
    self->num_color_targets = num_color_targets;
    self->color_targets = SDL_malloc(sizeof(SDL_GPUColorTargetDescription) * num_color_targets);
    if (self->color_targets == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "Failed to allocate color target descriptions");
        goto err;
    }
    SDL_memcpy(self->color_targets, color_targets, sizeof(SDL_GPUColorTargetDescription) * num_color_targets);

    return true;

err:
    shader__cleanup(self);
    return false;
}

bool shader__init_from_def(
    shader_t* self,
    SDL_GPUDevice* device,
    shader_def_t const* shader_def,
    size_t num_color_targets, SDL_GPUColorTargetDescription const* color_targets
) {
    return shader__init(
        self,
        device,
        *shader_def->vertex_src_len,
        shader_def->vertex_src,
        *shader_def->fragment_src_len,
        shader_def->fragment_src,
        shader_def->num_vertex_buffers,
        shader_def->vertex_buffers,
        shader_def->num_vertex_attributes,
        shader_def->vertex_attributes,
        num_color_targets,
        color_targets
    );
}

void shader__cleanup(shader_t* self) {
    // Delete pipelines
    for (size_t i = 0; i < self->pipelines_len; i++) {
        SDL_ReleaseGPUGraphicsPipeline(self->device, self->pipelines[i].pipeline);
    }
    SDL_free(self->pipelines);
    self->pipelines = nullptr;

    // Delete copied buffers
    SDL_free(self->color_targets);
    self->color_targets = nullptr;
    SDL_free(self->vertex_attributes);
    self->vertex_attributes = nullptr;
    SDL_free(self->vertex_buffers);
    self->vertex_buffers = nullptr;

    // Delete shaders
    SDL_ReleaseGPUShader(self->device, self->fragment_shader);
    self->fragment_shader = nullptr;
    SDL_ReleaseGPUShader(self->device, self->vertex_shader);

    self->device = nullptr;
}

bool shader__bind(shader_t* self, shader_config_t config, SDL_GPURenderPass* render_pass) {
    uint64_t config_hash = hash__fnv1a_64(FNV1_64A_INIT, sizeof(shader_config_t), &config);
    shader_pipeline_t* pipeline = SDL_bsearch(
        &config_hash,
        self->pipelines,
        self->pipelines_len,
        sizeof(shader_pipeline_t),
        compare_pipelines_by_hash
    );

    if (pipeline == nullptr) {
        pipeline = create_pipeline(self, &config, config_hash);
        if (pipeline == nullptr) {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "Could not create pipeline");
            return false;
        }
    }

    SDL_BindGPUGraphicsPipeline(render_pass, pipeline->pipeline);
    return true;
}

static int compare_pipelines_by_hash(void const* a, void const* b) {
    uint64_t const* hash_a = a;
    uint64_t const* hash_b = b;

    return (*hash_a > *hash_b) - (*hash_a < *hash_b);
}

static shader_pipeline_t* create_pipeline(shader_t* self, shader_config_t const* config, uint64_t config_hash) {
    shader_pipeline_t* out = nullptr;

    // Ensure pipeline buffer is large enough
    size_t pipeline_index = self->pipelines_len;
    if (self->pipelines_capacity == self->pipelines_len) {
        size_t new_pipelines_capacity = self->pipelines_capacity + PIPELINE_ALLOC_SIZE;
        shader_pipeline_t* new_pipelines = SDL_realloc(self->pipelines, sizeof(shader_config_t) * new_pipelines_capacity);
        if (new_pipelines == nullptr) {
            SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "Failed to allocate %zu pipelines", new_pipelines_capacity);
            goto end;
        }

        self->pipelines_capacity = new_pipelines_capacity;
        self->pipelines = new_pipelines;
    }

    // Duplicate color targets
    SDL_GPUColorTargetDescription* color_targets = SDL_malloc(sizeof(SDL_GPUColorTargetDescription) * self->num_color_targets);
    if (color_targets == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "Failed to allocate %zu color targets", self->num_color_targets);
        goto end;
    }
    SDL_memcpy(color_targets, self->color_targets, sizeof(SDL_GPUColorTargetDescription) * self->num_color_targets);

    // Create blend state
    SDL_GPUColorTargetBlendState blend_state = {0};
    if (config->blend) {
        blend_state = (SDL_GPUColorTargetBlendState) {
            .src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
            .dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
            .color_blend_op = SDL_GPU_BLENDOP_ADD,
            .src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
            .dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_DST_ALPHA,
            .alpha_blend_op = SDL_GPU_BLENDOP_ADD,
            .enable_blend = true,
        };
    }

    // Set up color target parameters
    for (size_t i = 0; i < self->num_color_targets; i++) {
        color_targets[i].blend_state = blend_state;
    }

    // Set up pipeline create info
    SDL_GPUGraphicsPipelineCreateInfo info = {
        .vertex_shader = self->vertex_shader,
        .fragment_shader = self->fragment_shader,
        .vertex_input_state = {
            .vertex_buffer_descriptions = self->vertex_buffers,
            .num_vertex_buffers = self->num_vertex_buffers,
            .vertex_attributes = self->vertex_attributes,
            .num_vertex_attributes = self->num_vertex_attributes,
        },
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .target_info = {
            .color_target_descriptions = color_targets,
            .num_color_targets = self->num_color_targets,
        }
    };

    // Create pipeline
    SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(
        self->device,
        &info
    );
    if (pipeline == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "Failed to create GPU pipeline: %s", SDL_GetError());
        goto end;
    }

    // Init pipeline object
    self->pipelines[pipeline_index].config_hash = config_hash;
    self->pipelines[pipeline_index].pipeline = pipeline;
    self->pipelines_len++;

    // Sort pipelines
    SDL_qsort(
        self->pipelines,
        self->pipelines_len,
        sizeof(shader_pipeline_t),
        compare_pipelines_by_hash
    );

    // Get pipeline back
    out = SDL_bsearch(
        &config_hash,
        self->pipelines,
        self->pipelines_len,
        sizeof(shader_pipeline_t),
        compare_pipelines_by_hash
    );
    SDL_assert(out != nullptr);

end:
    SDL_free(color_targets);
    return out;
}