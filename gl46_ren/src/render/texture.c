#include "./texture.h"

#include <SDL3/SDL_assert.h>

#include "logger.h"
#include "util/util.h"

bool texture__init(texture_t* self) {
    OBJECT_ZERO_INIT(self);

    return true;
}

void texture__cleanup(texture_t* self) {
    glDeleteTextures(1, &self->gl_texture);
    self->gl_texture = 0;
    
    self->width = 0;
    self->height = 0;
}

void texture__upload(texture_t* self, size_t width, size_t height, void* data) {
    if (self->width != width || self->height != height) {
        GLuint new_texture;
        glCreateTextures(GL_TEXTURE_2D, 1, &new_texture);
        if (new_texture == 0) {
            LOG_ERROR("Failed to recreate texture");
            return;
        }

        glDeleteTextures(1, &self->gl_texture);
        self->gl_texture = new_texture;

        glTextureStorage2D(new_texture, 1, GL_RGBA8, width, height);
        self->width = width;
        self->height = height;
    }

    glTextureSubImage2D(self->gl_texture, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

void texture__bind(texture_t const* self, size_t texture_unit) {
    SDL_assert(self->gl_texture != 0);

    glBindTextureUnit(texture_unit, self->gl_texture);
}
