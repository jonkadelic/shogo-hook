#include "./texture.h"

#include <SDL3/SDL_assert.h>

#include "logger.h"
#include "util/util.h"

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

void texture__upload(texture_t* self, size_t width, size_t height, size_t bpp, void* data) {
    GLenum internal_format = GL_RGBA8;
    GLenum format = GL_RGBA;
    switch (bpp) {
        case 1: {
            internal_format = GL_R8;
            format = GL_RED;
        } break;
        case 2: {
            internal_format = GL_RG8;
            format = GL_RG;
        } break;
        case 3: {
            internal_format = GL_RGB8;
            format = GL_RGB;
        } break;
        case 4: {
            internal_format = GL_RGBA8;
            format = GL_RGBA;
        } break;
        default: {
            LOG_WARNING("Attempted to upload to texture using invalid BPP %zu", bpp);
            return;
        }
    }

    if (self->width != width || self->height != height) {
        GLuint new_texture;
        glCreateTextures(GL_TEXTURE_2D, 1, &new_texture);
        if (new_texture == 0) {
            LOG_ERROR("Failed to recreate texture");
            return;
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

        glTextureStorage2D(new_texture, 1, internal_format, width, height);
        self->width = width;
        self->height = height;
    }

    glTextureSubImage2D(self->gl_texture, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);
}

void texture__upload_rect_buffer(texture_t* self, rect_buffer_t const* buffer) {
    texture__upload(self, buffer->width, buffer->height, buffer->bpp, buffer->data);
}

void texture__bind(texture_t const* self, size_t texture_unit) {
    SDL_assert(self->gl_texture != 0);

    glBindTextureUnit(texture_unit, self->gl_texture);
}
