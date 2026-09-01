#include "./pixel_buffer.h"

#include <SDL3/SDL.h>

#include "logger.h"
#include "renderer.h"
#include "util/util.h"

static bool pb_init(pixel_buffer_t* self, size_t width, size_t height, color_format_t format, bool create_framebuffer);
static void clamp_rect(rect_t* rect, rect_t const* bounds);

bool pixel_buffer__init(pixel_buffer_t* self, size_t width, size_t height, color_format_t format) {
    OBJECT_ZERO_INIT(self);

    if (!pb_init(self, width, height, format, true)) {
        goto err;
    }

    return true;

err:
    pixel_buffer__cleanup(self);
    return false;
}

bool pixel_buffer__init_backbuffer(pixel_buffer_t* self) {
    OBJECT_ZERO_INIT(self);

    auto window = renderer__get_instance()->window;

    int w, h;
    if (!SDL_GetWindowSize(window, &w, &h)) {
        LOG_ERROR("Could not read window size");
        goto err;
    }

    if (!pb_init(self, w, h, COLOR_FORMAT__RGBA32, false)) {
        goto err;
    }

    return true;

err:
    pixel_buffer__cleanup(self);
    return false;
}

void pixel_buffer__cleanup(pixel_buffer_t* self) {
    glDeleteFramebuffers(1, &self->gl_framebuffer);
    self->gl_framebuffer = 0;

    glDeleteTextures(1, &self->gl_texture);
    self->gl_texture = 0;

    glUnmapNamedBuffer(self->gl_pixel_buffer);
    self->pixel_data = nullptr;

    glDeleteBuffers(1, &self->gl_pixel_buffer);
    self->gl_pixel_buffer = 0;
}

void pixel_buffer__blit(pixel_buffer_t* self) {
    if (self->gl_pixel_buffer == 0) {
        return;
    }

    GLenum format, type;
    color_format__get_gl_enums(self->format, &format, nullptr, &type);

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, self->gl_pixel_buffer);
    glTextureSubImage2D(self->gl_texture, 0, 0, 0, self->width, self->height, format, type, nullptr);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

void pixel_buffer__clear(pixel_buffer_t* self, rect_t* opt_dst_rect, uint32_t clear_to) {
    if (self->gl_pixel_buffer == 0) {
        return;
    }

    if (clear_to == 0x00 && opt_dst_rect == nullptr) {
        SDL_memset(self->pixel_data, 0, self->width * self->height * self->bpp);
        return;
    }

    rect_t dst_rect = self->dims;
    if (opt_dst_rect != nullptr) {
        dst_rect = *opt_dst_rect;
    }
    clamp_rect(&dst_rect, &self->dims);
    if (dst_rect.x0 >= dst_rect.x1 || dst_rect.y0 >= dst_rect.y1) return;

    size_t width = dst_rect.x1 - dst_rect.x0;
    size_t height = dst_rect.y1 - dst_rect.y0;

    for (size_t y = dst_rect.y0; y < dst_rect.y1; y++) {
        for (size_t x = dst_rect.x0; x < dst_rect.x1; x++) {
            size_t i = (y * self->width * self->bpp) + (x * self->bpp);

            SDL_memcpy(self->pixel_data + i, &clear_to, self->bpp);
        }
    }
}

bool pixel_buffer__copy(pixel_buffer_t* self, pixel_buffer_t const* src, rect_t* opt_src_rect, rect_t* opt_dst_rect) {
    rect_t src_rect = src->dims;
    if (opt_src_rect != nullptr) {
        src_rect = *opt_src_rect;
    }
    clamp_rect(&src_rect, &src->dims);
    if (src_rect.x0 >= src_rect.x1 || src_rect.y0 >= src_rect.y1) return false;

    rect_t dst_rect = self->dims;
    if (opt_dst_rect != nullptr) {
        dst_rect = *opt_dst_rect;
    }
    clamp_rect(&dst_rect, &self->dims);
    if (dst_rect.x0 >= dst_rect.x1 || dst_rect.y0 >= dst_rect.y1) return false;

    size_t src_width = src_rect.x1 - src_rect.x0;
    size_t src_height = src_rect.y1 - src_rect.y0;
    size_t dst_width = dst_rect.x1 - dst_rect.x0;
    size_t dst_height = dst_rect.y1 - dst_rect.y0;

    if (self->gl_framebuffer == 0) {
        // need to flip backbuffer draws vertically
        glBlitNamedFramebuffer(
            src->gl_framebuffer, self->gl_framebuffer,
            src_rect.x0, src_rect.y0, src_rect.x1, src_rect.y1,
            dst_rect.x0, dst_rect.y1, dst_rect.x1, dst_rect.y0,
            GL_COLOR_BUFFER_BIT,
            GL_LINEAR
        );
    } else {
        glBlitNamedFramebuffer(
            src->gl_framebuffer, self->gl_framebuffer,
            src_rect.x0, src_rect.y0, src_rect.x1, src_rect.y1,
            dst_rect.x0, dst_rect.y0, dst_rect.x1, dst_rect.y1,
            GL_COLOR_BUFFER_BIT,
            GL_LINEAR
        );
    }

    return true;
}

static bool pb_init(pixel_buffer_t* self, size_t width, size_t height, color_format_t format, bool create_framebuffer) {
    if (width == 0 || height == 0 || format > NUM_COLOR_FORMATS) {
        return false;
    }

    self->width = width;
    self->height = height;
    self->format = format;
    self->bpp = color_format__get_bpp(format);
    self->dims = (rect_t) {
        .x0 = 0, .y0 = 0,
        .x1 = self->width, .y1 = self->height,
    };

    // Init PBO
    glCreateBuffers(1, &self->gl_pixel_buffer);
    if (self->gl_pixel_buffer == 0) {
        LOG_ERROR("Failed to create PBO for pixel buffer");
        return false;
    }
    glNamedBufferStorage(self->gl_pixel_buffer, width * height * self->bpp, nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

    // Map PBO data
    self->pixel_data = glMapNamedBufferRange(self->gl_pixel_buffer, 0, width * height * self->bpp, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    if (self->pixel_data == nullptr) {
        LOG_ERROR("Failed to map PBO for pixel buffer");
        return false;
    }

    // Init texture
    GLenum gl_internal_format;
    color_format__get_gl_enums(format, nullptr, &gl_internal_format, nullptr);
    glCreateTextures(GL_TEXTURE_2D, 1, &self->gl_texture);
    if (self->gl_texture == 0) {
        LOG_ERROR("Failed to create texture for pixel buffer");
        return false;
    }
    glTextureStorage2D(self->gl_texture, 1, gl_internal_format, width, height);

    // Init framebuffer
    if (create_framebuffer) {
        glCreateFramebuffers(1, &self->gl_framebuffer);
        if (self->gl_framebuffer == 0) {
            LOG_ERROR("Failed to create framebuffer for pixel buffer");
            return false;
        }
        glNamedFramebufferTexture(self->gl_framebuffer, GL_COLOR_ATTACHMENT0, self->gl_texture, 0);
    
        // Check framebuffer
        GLenum status = glCheckNamedFramebufferStatus(self->gl_framebuffer, GL_READ_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            LOG_ERROR("Framebuffer not readable");
            return false;
        }
        status = glCheckNamedFramebufferStatus(self->gl_framebuffer, GL_DRAW_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            LOG_ERROR("Framebuffer not drawable");
            return false;
        }
    } else {
        self->gl_framebuffer = 0; // backbuffer framebuffer
    }

    return true;
}

static void clamp_rect(rect_t* rect, rect_t const* bounds) {
    rect->x0 = SDL_max(rect->x0, bounds->x0);
    rect->y0 = SDL_max(rect->y0, bounds->y0);
    rect->x1 = SDL_min(rect->x1, bounds->x1);
    rect->y1 = SDL_min(rect->y1, bounds->y1);
}
