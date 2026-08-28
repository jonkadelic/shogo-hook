#include "./blitter.h"

#include <assert.h>
#include <minwindef.h>
#include <stdio.h>
#include <stdlib.h>

#include "render/surface.h"
#include "util/debug.h"
#include "util/util.h"

static void update_surface_texture(surface_t* surface, bool transparency, uint16_t transparent_color);

static void export_ppm(surface_t* surface, void* buffer);

bool blitter__init(blitter_t* self, SDL_GPUDevice* device, tessellator_t* tessellator) {
    ZERO_INIT_STRUCT(self);

    self->device = device;

    if (!mesh__init(&self->mesh, device)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to init mesh");
        goto err;
    }

    auto tbuf = tessellator__start(tessellator, &self->mesh);
    if (tbuf == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to start tessellating");
        goto err;
    }

    // Tessellate mesh data
    tessellator_buffer__append_vertices(tbuf,
        4,
        (vertex_t[4]) {
            (vertex_t) { 
                .x = 0.0f, .y = 0.0f, .z = 0.0f,
                .r = 1.0f, .g = 0.0f, .b = 0.0f, .a = 1.0f,
            },
            (vertex_t) {
                .x = 1.0f, .y = 0.0f, .z = 0.0f,
                .r = 0.0f, .g = 1.0f, .b = 0.0f, .a = 1.0f,
            },
            (vertex_t) {
                .x = 0.0f, .y = 1.0f, .z = 0.0f,
                .r = 0.0f, .g = 1.0f, .b = 0.0f, .a = 1.0f,
            },
            (vertex_t) {
                .x = 1.0f, .y = 1.0f, .z = 0.0f,
                .r = 1.0f, .g = 0.0f, .b = 1.0f, .a = 1.0f,
            }
        }
    );
    tessellator_buffer__append_indices(tbuf,
        6,
        (index_t[6]) {
            0, 2, 1,
            1, 2, 3
        }
    );
    tessellator_buffer__finish(tbuf);

    return true;

err:
    blitter__cleanup(self);
    return false;
}

void blitter__cleanup(blitter_t* self) {
    mesh__cleanup(&self->mesh);
}

void blit__blit_to_screen(blitter_t* self, BlitRequest_t const* request, SDL_GPURenderPass* render_pass) {
    DEBUG_PRINT_FUN();

    if (request->m_BlitOptions == 0x04) {
        return;
    }

    surface_t* surface = request->m_pSurface;
    DRect_t* dest = request->m_pDestRect;

    if (surface->locked || surface->glTexture == 0) {
        update_surface_texture(surface, (request->m_BlitOptions & 0x01) != 0, request->m_TransparentColor);
    }

    if ((request->m_BlitOptions & 0x01) != 0) {
    //     glEnable(GL_BLEND);
    //     glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //     glEnable(GL_ALPHA_TEST);
    } else {
    //     glDisable(GL_BLEND);
    }

    // glDisable(GL_DEPTH_TEST);
    // glDepthMask(0);
    // glEnable(GL_TEXTURE_2D);

    // glBindTexture(GL_TEXTURE_2D, surface->glTexture);

    float u_max = surface->width / (float) surface->glWidth;
    float v_max = surface->height / (float) surface->glHeight;

    // glBegin(GL_QUADS);
    // glColor3f(1.0f, 1.0f, 1.0f);
    // glTexCoord2f(0.0f, 0.0f);
    // glVertex2f(dest->left, dest->top);
    // glTexCoord2f(u_max, 0.0f);
    // glVertex2f(dest->right, dest->top);
    // glTexCoord2f(u_max, v_max);
    // glVertex2f(dest->right, dest->bottom);
    // glTexCoord2f(0.0f, v_max);
    // glVertex2f(dest->left, dest->bottom);
    // glEnd();
}

static void update_surface_texture(surface_t* surface, bool transparency, uint16_t transparent_color) {
    DEBUG_PRINT_FUN();

    if (surface->glTexture == 0) {
        surface->glWidth = 0;
        surface->glHeight = 0;

        return;

        // glGenTextures(1, &surface->glTexture);
        DEBUG_ASSERT(surface->glTexture != 0);

        size_t size_u, size_v;
        for (size_u = 0x4000; size_u != surface->width; size_u /= 2) {
            if (size_u == 1) {
                size_u = 2;
                break;
            }
            if (size_u < surface->width) {
                size_u *= 2;
                break;
            }
        }
        surface->glWidth = size_u;

        for (size_v = 0x4000; size_v != surface->height; size_v /= 2) {
            if (size_v == 1) {
                size_v = 2;
                break;
            }
            if (size_v < surface->height) {
                size_v *= 2;
                break;
            }
        }
        surface->glHeight = size_v;
        
    }

    uint32_t* buffer = malloc(surface->glWidth * surface->glHeight * 4);
    uint16_t* data = (uint16_t*) surface->data;

    if (buffer != nullptr && surface->height > 0) {
        for (size_t y = 0; y < surface->height; y++) {
            for (size_t x = 0; x < surface->width; x++) {
                size_t i = (y * surface->width) + x;
                uint8_t red = (data[i] & 0xF800) >> 11;
                uint8_t green = (data[i] & 0x07C0) >> 6;
                uint8_t blue = (data[i] & 0x003F) >> 1;

                if (transparency && data[i] == transparent_color) {
                    buffer[(y * surface->glWidth) + x] = 0x00000000;
                } else {
                    buffer[(y * surface->glWidth) + x] = 0xFF000000 | (red << 3) | ((green << 3) << 8) | ((blue << 3) << 16);
                }
            }
        }

        export_ppm(surface, buffer);

        // glBindTexture(GL_TEXTURE_2D, surface->glTexture);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->glWidth, surface->glHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    }

    free(buffer);
    surface->locked = false;
}

static void export_ppm(surface_t* surface, void* buffer) {
    char path[MAX_PATH];
    snprintf(path, sizeof(path), "surface%u.ppm", surface->glTexture);

    FILE* file = fopen(path, "wb");
    if (file == nullptr) {
        return;
    }

    uint8_t* buf = buffer;

    fprintf(file, "P3\n%u %u\n255\n", surface->glWidth, surface->glHeight);
    for (size_t y = 0; y < surface->glHeight; y++) {
        for (size_t x = 0; x < surface->glWidth; x++) {
            size_t i = (y * surface->glWidth * 4) + x * 4;
            fprintf(file, "%hhu %hhu %hhu\n", buf[i], buf[i + 1], buf[i + 2]);
        }
    }

    fclose(file);
}