#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct rect {
    size_t x0, y0;
    size_t x1, y1;
} rect_t;

typedef struct rect_buffer {
    size_t width;
    size_t height;
    size_t bpp;
    rect_t dims;
    uint8_t* data;
} rect_buffer_t;

bool rect_buffer__init(rect_buffer_t* self, size_t width, size_t height, size_t bpp);
void rect_buffer__cleanup(rect_buffer_t* self);

void rect_buffer__clear(rect_buffer_t* self, rect_t* opt_dst_rect, size_t clear_to);
bool rect_buffer__copy(rect_buffer_t* self, rect_buffer_t const* src, rect_t* opt_src_rect, rect_t* opt_dst_rect);
