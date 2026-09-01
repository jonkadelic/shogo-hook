#include "./pixel_buffer.h"

#include <SDL3/SDL.h>

#include "logger.h"
#include "util/util.h"

static void clamp_rect(rect_t* rect, rect_t const* bounds);

bool pixel_buffer__init(pixel_buffer_t* self, size_t width, size_t height, color_format_t format) {
    OBJECT_ZERO_INIT(self);

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
        goto err;
    }
    glNamedBufferStorage(self->gl_pixel_buffer, width * height * self->bpp, nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

    // Map PBO data
    self->data = glMapNamedBufferRange(self->gl_pixel_buffer, 0, width * height * self->bpp, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    if (self->data == nullptr) {
        LOG_ERROR("Failed to map PBO for pixel buffer");
        goto err;
    }

    // Init texture
    GLenum gl_internal_format;
    color_format__get_gl_enums(format, nullptr, &gl_internal_format, nullptr);
    glCreateTextures(GL_TEXTURE_2D, 1, &self->gl_texture);
    if (self->gl_texture == 0) {
        LOG_ERROR("Failed to create texture for pixel buffer");
        goto err;
    }
    glTextureStorage2D(self->gl_texture, 1, gl_internal_format, width, height);

    // Init framebuffer
    glCreateFramebuffers(1, &self->gl_framebuffer);
    if (self->gl_framebuffer == 0) {
        LOG_ERROR("Failed to create framebuffer for pixel buffer");
        goto err;
    }
    glNamedFramebufferTexture(self->gl_framebuffer, GL_COLOR_ATTACHMENT0, self->gl_texture, 0);

    // Check framebuffer
    GLenum status = glCheckNamedFramebufferStatus(self->gl_framebuffer, GL_READ_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        LOG_ERROR("Framebuffer not readable");
        goto err;
    }
    status = glCheckNamedFramebufferStatus(self->gl_framebuffer, GL_DRAW_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        LOG_ERROR("Framebuffer not drawable");
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
    self->data = nullptr;

    glDeleteBuffers(1, &self->gl_pixel_buffer);
    self->gl_pixel_buffer = 0;
}

void pixel_buffer__blit(pixel_buffer_t* self) {
    GLenum format, type;
    color_format__get_gl_enums(self->format, &format, nullptr, &type);

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, self->gl_pixel_buffer);
    glTextureSubImage2D(self->gl_texture, 0, 0, 0, self->width, self->height, format, type, nullptr);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

void pixel_buffer__clear(pixel_buffer_t* self, rect_t* opt_dst_rect, uint32_t clear_to) {
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

            SDL_memcpy(self->data + i, &clear_to, self->bpp);
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

    uint64_t ticks_start = SDL_GetTicks();
    if (src_width == dst_width && src_height == dst_height) {
        // Do a fast copy (no scaling)
        for (size_t y = 0; y < dst_height; y++) {
            for (size_t x = 0; x < dst_width; x++) {
                size_t src_i = ((y + src_rect.y0) * (src->width * src->bpp)) + ((x + src_rect.x0) * src->bpp);
                size_t dst_i = ((y + dst_rect.y0) * (self->width * self->bpp)) + ((x + dst_rect.x0) * self->bpp);

                auto converted = color_format__convert_color(src->format, self->format, *(uint32_t*) (src->data + src_i));
                SDL_memcpy(self->data + dst_i, &converted, self->bpp);
            }
        }
    } else {
        // Do a slow copy (rescaling)
        for (size_t dy = 0; dy < dst_height; dy++) {
            for (size_t dx = 0; dx < dst_width; dx++) {
                float px = (float) dx / (float) dst_width;
                float py = (float) dy / (float) dst_height;
                size_t sx = (size_t) (px * src_width);
                size_t sy = (size_t) (py * src_height);

                size_t src_i = ((sy + src_rect.y0) * (src->width * src->bpp)) + ((sx + src_rect.x0) * src->bpp);
                size_t dst_i = ((dy + dst_rect.y0) * (self->width * self->bpp)) + ((dx + dst_rect.x0) * self->bpp);

                auto converted = color_format__convert_color(src->format, self->format, *(uint32_t*) (src->data + src_i));
                SDL_memcpy(self->data + dst_i, &converted, self->bpp);
            }
        }
    }
    LOG_INFO("Buffer copy %zu*%zu to %zu*%zu took %llums", src_width, src_height, dst_width, dst_height, SDL_GetTicks() - ticks_start);

    return true;
}

static void clamp_rect(rect_t* rect, rect_t const* bounds) {
    rect->x0 = SDL_max(rect->x0, bounds->x0);
    rect->y0 = SDL_max(rect->y0, bounds->y0);
    rect->x1 = SDL_min(rect->x1, bounds->x1);
    rect->y1 = SDL_min(rect->y1, bounds->y1);
}
