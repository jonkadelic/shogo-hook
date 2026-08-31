#include "./rect_buffer.h"

#include <SDL3/SDL.h>

#include "logger.h"
#include "util/util.h"

static void clamp_rect(rect_t* rect, rect_t const* bounds);

bool rect_buffer__init(rect_buffer_t* self, size_t width, size_t height, size_t bpp) {
    OBJECT_ZERO_INIT(self);

    if (width == 0 || height == 0 || bpp == 0 || bpp > 4) {
        return false;
    }

    self->width = width;
    self->height = height;
    self->bpp = bpp;
    self->dims = (rect_t) {
        .x0 = 0, .y0 = 0,
        .x1 = self->width, .y1 = self->height,
    };

    self->data = SDL_calloc(1, width * height * bpp);
    if (self->data == nullptr) {
        LOG_ERROR("Failed to alloc %zu*%zu*%zu rect buffer", width, height, bpp);
        return false;
    }

    return true;
}

void rect_buffer__cleanup(rect_buffer_t* self) {
    SDL_free(self->data); self->data = nullptr;
}

void rect_buffer__clear(rect_buffer_t* self, rect_t* opt_dst_rect, uint32_t clear_to) {
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

bool rect_buffer__copy(rect_buffer_t* self, rect_buffer_t const* src, rect_t* opt_src_rect, rect_t* opt_dst_rect) {
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

    size_t target_bpp = SDL_min(self->bpp, src->bpp);

    if ((src_rect.x1 - src_rect.x0) == (dst_rect.x1 - dst_rect.x0) && (src_rect.y1 - src_rect.y0) == (dst_rect.y1 - dst_rect.y0)) {
        // Do a fast copy (no scaling)
        size_t width = dst_rect.x1 - dst_rect.x0;
        size_t height = dst_rect.y1 - dst_rect.y0;

        for (size_t y = 0; y < height; y++) {
            for (size_t x = 0; x < width; x++) {
                size_t src_i = ((y + src_rect.y0) * (src->width * src->bpp)) + ((x + src_rect.x0) * src->bpp);
                size_t dst_i = ((y + dst_rect.y0) * (self->width * self->bpp)) + ((x + dst_rect.x0) * self->bpp);

                SDL_memset(self->data + dst_i, 0xFF, self->bpp);
                SDL_memcpy(self->data + dst_i, src->data + src_i, target_bpp);
            }
        }
    } else {
        // Do a slow copy (rescaling)
        LOG_WARNING("Scaling copy not yet implemented!");
        return true;
    }

    return true;
}

static void clamp_rect(rect_t* rect, rect_t const* bounds) {
    rect->x0 = SDL_max(rect->x0, bounds->x0);
    rect->y0 = SDL_max(rect->y0, bounds->y0);
    rect->x1 = SDL_min(rect->x1, bounds->x1);
    rect->y1 = SDL_min(rect->y1, bounds->y1);
}