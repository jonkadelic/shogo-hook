#include "./shader.h"

#include <SDL3/SDL_assert.h>

#include "logger.h"
#include "util/util.h"
#include "util/hash.h"
#include "shaders/shaders.h"

// name_hash must be first member for `compare_uniforms_by_hash` to work
static_assert(offsetof(shader_uniform_t, name_hash) == 0);

// location must be first member for `compare_samplers_by_location` to work
static_assert(offsetof(shader_sampler_t, location) == 0);

static int compare_uniforms_by_hash(void const* a, void const* b);
static int compare_samplers_by_location(void const* a, void const* b);

static GLint BOUND_PROGRAM = 0;

bool shader__init(shader_t* self, char const* opt_name, char const* const shader_srcs[NUM_SHADER_SRC_TYPES]) {
    OBJECT_ZERO_INIT(self);

    GLuint gl_shaders[NUM_SHADER_SRC_TYPES] = {};

    // Create shader program
    self->gl_program = glCreateProgram();
    if (self->gl_program == 0) {
        LOG_ERROR("Failed to create shader program");
        goto err;
    }

    // Set up label if provided
    if (opt_name != nullptr) {
        glObjectLabel(GL_PROGRAM, self->gl_program, -1, opt_name);
    }

    // Iterate and compile each shader source type
    for (shader_src_type_t i = 0; i < NUM_SHADER_SRC_TYPES; i++) {
        if (shader_srcs[i] == nullptr) {
            continue;
        }

        GLenum shader_type;
        switch (i) {
            case SHADER_SRC_TYPE__VERTEX: {
                shader_type = GL_VERTEX_SHADER;
            } break;
            case SHADER_SRC_TYPE__FRAGMENT: {
                shader_type = GL_FRAGMENT_SHADER;
            } break;
            default: {
                SDL_assert(false);
            }
        }

        // Create shader
        gl_shaders[i] = glCreateShader(shader_type);
        if (gl_shaders[i] == 0) {
            LOG_ERROR("Failed to create shader");
            goto err;
        }

        // Compile shader
        glShaderSource(gl_shaders[i], 1, &shader_srcs[i], nullptr);
        glCompileShader(gl_shaders[i]);

        // Check shader compilation result
        GLint compile_status = GL_FALSE;
        glGetShaderiv(gl_shaders[i], GL_COMPILE_STATUS, &compile_status);
        if (compile_status != GL_TRUE) {
            GLint info_log_len = 0;
            glGetShaderiv(gl_shaders[i], GL_INFO_LOG_LENGTH, &info_log_len);
            if (info_log_len == 0) {
                LOG_ERROR("Could not compile shader: unknown error");
                goto err;
            }

            char* log = SDL_malloc(sizeof(char) * info_log_len);
            if (log == nullptr) {
                LOG_ERROR("Could not compile shader: unknown error");
                goto err;
            }

            glGetShaderInfoLog(gl_shaders[i], info_log_len, nullptr, log);
            LOG_ERROR("Could not compile shader:");
            LOG_ERROR("%s", log);

            SDL_free(log);
            goto err;
        }

        // Attach shader to main program
        glAttachShader(self->gl_program, gl_shaders[i]);
    }

    // Link shader program
    glLinkProgram(self->gl_program);

    // Check program link result
    GLint link_status = GL_FALSE;
    glGetProgramiv(self->gl_program, GL_LINK_STATUS, &link_status);
    if (link_status != GL_TRUE) {
        GLint info_log_len = 0;
        glGetProgramiv(self->gl_program, GL_INFO_LOG_LENGTH, &info_log_len);
        if (info_log_len == 0) {
            LOG_ERROR("Could not link program: unknown error");
            goto err;
        }

        char* log = SDL_malloc(sizeof(char) * info_log_len);
        if (log == nullptr) {
            LOG_ERROR("Could not link program: unknown error");
            goto err;
        }

        glGetProgramInfoLog(self->gl_program, info_log_len, nullptr, log);
        LOG_ERROR("Could not link program:");
        LOG_ERROR("%s", log);

        SDL_free(log);
        goto err;
    }

    // Delete shaders (no longer needed)
    for (shader_src_type_t i = 0; i < NUM_SHADER_SRC_TYPES; i++) {
        glDeleteShader(gl_shaders[i]);
    }

    GLuint tex_unit = 0;

    // Get number of uniforms in program
    GLint uniform_count = 0;
    glGetProgramiv(self->gl_program, GL_ACTIVE_UNIFORMS, &uniform_count);

    // Fill out uniform data
    if (uniform_count > 0) {
        GLsizei size = 0;
        GLenum type = GL_NONE;
        GLint max_name_len = 0;
        glGetProgramiv(self->gl_program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_name_len);

        self->uniforms = SDL_calloc(uniform_count, sizeof(shader_uniform_t));
        if (self->uniforms == nullptr) {
            goto err;
        }
        self->num_uniforms = uniform_count;

        self->samplers = SDL_calloc(uniform_count, sizeof(shader_sampler_t));
        if (self->samplers == nullptr) {
            goto err;
        }

        char* uniform_name = SDL_malloc(sizeof(char) * max_name_len);
        if (uniform_name == nullptr) {
            goto err;
        }

        for (GLint i = 0; i < uniform_count; i++) {
            glGetActiveUniform(self->gl_program, i, max_name_len, nullptr, &size, &type, uniform_name);

            GLint location = glGetUniformLocation(self->gl_program, uniform_name);

            self->uniforms[i].name_hash = hash__fnv1a_64(FNV1_64A_INIT, strlen(uniform_name), uniform_name);
            strncpy(self->uniforms[i].name, uniform_name, sizeof(self->uniforms[i].name));
            self->uniforms[i].location = location;

            if (type == GL_SAMPLER_2D) {
                glProgramUniform1i(self->gl_program, location, tex_unit);

                self->samplers[self->num_samplers].location = location;
                self->samplers[self->num_samplers].texture_unit = tex_unit;

                tex_unit++;
                self->num_samplers++;
            }
        }

        SDL_free(uniform_name);
    }

    // Sort uniforms by hash
    SDL_qsort(self->uniforms, self->num_uniforms, sizeof(shader_uniform_t), compare_uniforms_by_hash);

    // Sort samplers by location
    SDL_qsort(self->samplers, self->num_samplers, sizeof(shader_sampler_t), compare_samplers_by_location);

    return true;

err:
    for (shader_src_type_t i = 0; i < NUM_SHADER_SRC_TYPES; i++) {
        glDeleteShader(gl_shaders[i]);
    }

    shader__cleanup(self);
    return false;
}

bool shader__init_from_def(shader_t* self, shader_def_t const* def) {
    return shader__init(self, def->name, def->shader_srcs);
}

void shader__cleanup(shader_t* self) {
    SDL_free(self->samplers); self->samplers = nullptr;
    SDL_free(self->uniforms); self->uniforms = nullptr;
    glDeleteProgram(self->gl_program); self->gl_program = 0;
}

void shader__bind(shader_t const* self) {
    SDL_assert(self->gl_program != 0);

    if (BOUND_PROGRAM != self->gl_program) {
        glUseProgram(self->gl_program);
        BOUND_PROGRAM = self->gl_program;
    }
}

void shader__set_uniform_texture(shader_t const* self, char const* name, texture_t const* texture) {
    shader__set_uniform_texture_raw(self, name, texture->gl_texture);
}

void shader__set_uniform_texture_raw(shader_t const* self, char const* name, GLuint gl_texture) {
    uint64_t name_hash = hash__fnv1a_64(FNV1_64A_INIT, strlen(name), name);
    shader_uniform_t* uniform = SDL_bsearch(&name_hash, self->uniforms, self->num_uniforms, sizeof(shader_uniform_t), compare_uniforms_by_hash);
    if (uniform == nullptr) {
        LOG_WARNING("Attempted to bind to invalid uniform \"%s\"", name);
        return;
    }

    if (gl_texture == 0) {
        LOG_WARNING("Attempted to bind invalid texture");
        return;
    }

    shader_sampler_t* sampler = SDL_bsearch(&uniform->location, self->samplers, self->num_samplers, sizeof(shader_sampler_t), compare_samplers_by_location);
    if (sampler == nullptr) {
        LOG_ERROR("Found a uniform sampler with no texture unit - what gives?");
        return;
    }

    glBindTextureUnit(sampler->texture_unit, gl_texture);
}

void shader__set_uniform_mat4f(shader_t const* self, char const* name, HMM_Mat4 const* mat4) {
    uint64_t name_hash = hash__fnv1a_64(FNV1_64A_INIT, strlen(name), name);
    shader_uniform_t* uniform = SDL_bsearch(&name_hash, self->uniforms, self->num_uniforms, sizeof(shader_uniform_t), compare_uniforms_by_hash);
    if (uniform == nullptr) {
        LOG_WARNING("Attempted to bind to invalid uniform \"%s\"", name);
        return;
    }

    glProgramUniformMatrix4fv(self->gl_program, uniform->location, 1, false, (float const*) mat4->Elements);
}

static int compare_uniforms_by_hash(void const* a, void const* b) {
    uint64_t const* hash_a = a;
    uint64_t const* hash_b = b;

    return (*hash_a > *hash_b) - (*hash_a < *hash_b);
}

static int compare_samplers_by_location(void const* a, void const* b) {
    GLint const* location_a = a;
    GLint const* location_b = b;

    return (*location_a > *location_b) - (*location_a < *location_b);
}
