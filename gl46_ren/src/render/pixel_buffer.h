#pragma once

#include <stddef.h>
#include <stdint.h>

#include <glad/gl.h>

#include "util/color_format.h"

typedef struct rect {
    size_t x0, y0;
    size_t x1, y1;
} rect_t;

typedef struct pixel_buffer {
    size_t width;
    size_t height;
    color_format_t format;
    uint32_t bpp;
    rect_t dims;

    GLuint gl_pixel_buffer;
    GLuint gl_texture;
    GLuint gl_framebuffer;
    void* data;
} pixel_buffer_t;

bool pixel_buffer__init(pixel_buffer_t* self, size_t width, size_t height, color_format_t format);
void pixel_buffer__cleanup(pixel_buffer_t* self);

void pixel_buffer__blit(pixel_buffer_t* self);

void pixel_buffer__clear(pixel_buffer_t* self, rect_t* opt_dst_rect, uint32_t clear_to);
bool pixel_buffer__copy(pixel_buffer_t* self, pixel_buffer_t const* src, rect_t* opt_src_rect, rect_t* opt_dst_rect);
