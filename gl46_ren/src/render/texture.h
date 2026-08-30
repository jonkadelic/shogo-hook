#pragma once

#include <stddef.h>

#include <glad/gl.h>

typedef struct texture {
    GLuint gl_texture;
    size_t width; 
    size_t height;
} texture_t;

bool texture__init(texture_t* self);
void texture__cleanup(texture_t* self);

void texture__upload(texture_t* self, size_t width, size_t height, void* data);

void texture__bind(texture_t const* self, size_t texture_unit);
