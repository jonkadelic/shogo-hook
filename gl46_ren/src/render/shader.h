#pragma once

#include <glad/gl.h>
#include <hmm/hmm.h>

#include "render/texture.h"

typedef struct shader_def shader_def_t;

typedef enum shader_src_type {
    SHADER_SRC_TYPE__VERTEX,
    SHADER_SRC_TYPE__FRAGMENT,
    NUM_SHADER_SRC_TYPES,
} shader_src_type_t;

typedef struct shader_uniform {
    uint64_t name_hash;
    char name[32];
    GLint location;
} shader_uniform_t;

typedef struct shader_sampler {
    GLint location;
    GLuint texture_unit;
} shader_sampler_t;

typedef struct shader {
    GLuint gl_program;

    size_t num_uniforms;
    shader_uniform_t* uniforms;
    size_t num_samplers;
    shader_sampler_t* samplers;
} shader_t;

bool shader__init(shader_t* self, char const* opt_name, char const* const shader_srcs[NUM_SHADER_SRC_TYPES]);
bool shader__init_from_def(shader_t* self, shader_def_t const* def);
void shader__cleanup(shader_t* self);

void shader__bind(shader_t const* self);

void shader__set_uniform_texture(shader_t const* self, char const* name, texture_t const* texture);
void shader__set_uniform_texture_raw(shader_t const* self, char const* name, GLuint gl_texture);
void shader__set_uniform_mat4f(shader_t const* self, char const* name, HMM_Mat4 const* mat4);
