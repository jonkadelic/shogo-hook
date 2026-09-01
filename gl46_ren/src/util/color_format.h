#pragma once

#include <stdint.h>

#include <SDL3/SDL_assert.h>
#include <glad/gl.h>

typedef enum color_format {
    COLOR_FORMAT__RGBA32,
    COLOR_FORMAT__RGB565,
    NUM_COLOR_FORMATS
} color_format_t;

static inline uint32_t color_format__get_bpp(color_format_t self) {
    switch (self) {
        case COLOR_FORMAT__RGBA32: {
            return sizeof(uint32_t);
        } break;
        case COLOR_FORMAT__RGB565: {
            return sizeof(uint16_t);
        } break;
        default: {
            SDL_assert(false);
        }
    }
}

static inline uint32_t color_format__convert_color(color_format_t src, color_format_t target, uint32_t value) {
    if (src == target) return value;

    uint8_t r, g, b, a;

    switch (src) {
        case COLOR_FORMAT__RGBA32: {
            r = value & 0xFF;
            g = (value >> 8) & 0xFF;
            b = (value >> 16) & 0xFF;
            a = (value >> 24) & 0xFF;
        } break;
        case COLOR_FORMAT__RGB565: {
            r = (value & 0xF800) >> 11;
            r = (r << 3) | (r >> 2);
            g = (value & 0x07E0) >> 5;
            g = (g << 2) | (g >> 4);
            b = (value & 0x001F);
            b = (b << 3) | (b >> 2);
            a = 0xFF;
        } break;
        default: {
            SDL_assert(false);
        }
    }

    switch (target) {
        case COLOR_FORMAT__RGBA32: {
            return (a << 24) | (b << 16) | (g << 8) | r;
        } break;
        case COLOR_FORMAT__RGB565: {
            return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
        } break;
        default: {
            SDL_assert(false);
        }
    }
}

static inline void color_format__get_gl_enums(color_format_t self, GLenum* out_format, GLenum* out_internal_format, GLenum* out_type) {
    GLenum format, internal_format, type;

    switch (self) {
        case COLOR_FORMAT__RGBA32: {
            format = GL_RGBA;
            internal_format = GL_RGBA8;
            type = GL_UNSIGNED_BYTE;
        } break;
        case COLOR_FORMAT__RGB565: {
            format = GL_RGB;
            internal_format = GL_RGB565;
            type = GL_UNSIGNED_SHORT_5_6_5;
        } break;
        default: {
            SDL_assert(false);
        }
    }

    if (out_format != nullptr) {
        *out_format = format;
    }
    if (out_internal_format != nullptr) {
        *out_internal_format = internal_format;
    }
    if (out_type != nullptr) {
        *out_type = type;
    }
}