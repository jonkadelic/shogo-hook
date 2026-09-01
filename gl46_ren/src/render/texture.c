#include "./texture.h"

#include <SDL3/SDL_assert.h>

#include "logger.h"
#include "util/util.h"

static bool setup_gl_texture(texture_t* self, size_t width, size_t height, color_format_t format, GLenum* out_gl_format, GLenum* out_gl_type);

bool texture__init(texture_t* self, char const* opt_name) {
    OBJECT_ZERO_INIT(self);

    self->opt_name = opt_name;

    return true;
}

void texture__cleanup(texture_t* self) {
    glDeleteTextures(1, &self->gl_texture);
    self->gl_texture = 0;
    
    self->width = 0;
    self->height = 0;
}

void texture__upload(texture_t* self, size_t width, size_t height, color_format_t format, void* data) {
    GLenum gl_format;
    GLenum gl_type;

    if (!setup_gl_texture(self, width, height, format, &gl_format, &gl_type)) {
        LOG_ERROR("Failed to set up GL texture");
        return;
    }
    
    glTextureSubImage2D(self->gl_texture, 0, 0, 0, width, height, gl_format, gl_type, data);
}

void texture__upload_pixel_buffer(texture_t* self, pixel_buffer_t const* buffer) {
    GLenum gl_format;
    GLenum gl_type;

    if (!setup_gl_texture(self, buffer->width, buffer->height, buffer->format, &gl_format, &gl_type)) {
        LOG_ERROR("Failed to set up GL texture");
        return;
    }

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer->gl_pixel_buffer);
    glTextureSubImage2D(self->gl_texture, 0, 0, 0, self->width, self->height, gl_format, gl_type, nullptr);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);    
}

void texture__bind(texture_t const* self, size_t texture_unit) {
    SDL_assert(self->gl_texture != 0);

    glBindTextureUnit(texture_unit, self->gl_texture);
}

static bool setup_gl_texture(texture_t* self, size_t width, size_t height, color_format_t format, GLenum* out_gl_format, GLenum* out_gl_type) {
    GLenum gl_internal_format;
    GLenum gl_format;
    GLenum gl_type;

    switch (format) {
        case COLOR_FORMAT__RGBA32: {
            gl_internal_format = GL_RGBA8;
            gl_format = GL_RGBA;
            gl_type = GL_UNSIGNED_BYTE;
        } break;
        case COLOR_FORMAT__RGB565: {
            gl_internal_format = GL_RGB565;
            gl_format = GL_RGB;
            gl_type = GL_UNSIGNED_SHORT_5_6_5;
        } break;
        default: {
            LOG_WARNING("Attempted to upload to texture using invalid format %zu", (size_t) format);
            return false;
        }
    }

    if (self->width != width || self->height != height) {
        GLuint new_texture;
        glCreateTextures(GL_TEXTURE_2D, 1, &new_texture);
        if (new_texture == 0) {
            LOG_ERROR("Failed to recreate texture");
            return false;
        }

        if (self->opt_name != nullptr) {
            glObjectLabel(GL_TEXTURE, new_texture, -1, self->opt_name);
        }

        glTextureParameteri(new_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(new_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(new_texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(new_texture, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glDeleteTextures(1, &self->gl_texture);
        self->gl_texture = new_texture;

        glTextureStorage2D(new_texture, 1, gl_internal_format, width, height);
        self->width = width;
        self->height = height;
    }

    *out_gl_format = gl_format;
    *out_gl_type = gl_type;
    return true;
}