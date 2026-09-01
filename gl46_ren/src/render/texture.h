#pragma once

#include <stddef.h>

#include <glad/gl.h>

#include "render/pixel_buffer.h"

typedef struct texture {
    char const* opt_name;
    GLuint gl_texture;
    size_t width; 
    size_t height;
} texture_t;

bool texture__init(texture_t* self, char const* opt_name);
void texture__cleanup(texture_t* self);

void texture__upload(texture_t* self, size_t width, size_t height, color_format_t format, void* data);
void texture__upload_pixel_buffer(texture_t* self, pixel_buffer_t const* buffer);

void texture__bind(texture_t const* self, size_t texture_unit);
