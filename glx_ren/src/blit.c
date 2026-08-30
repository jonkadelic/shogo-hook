#include "./blit.h"

#include <assert.h>
#include <minwindef.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL3/SDL_stdinc.h>

#include "surfaces.h"
#include "debug.h"

static void update_surface_texture(surface_t* surface, bool transparency, uint16_t transparent_color);

static void export_ppm(surface_t* surface, void* buffer);

void blit__blit_to_screen(BlitRequest_t* request) {
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
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_ALPHA_TEST);
    } else {
        glDisable(GL_BLEND);
    }

    glDisable(GL_DEPTH_TEST);
    glDepthMask(0);
    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, surface->glTexture);

    float u_max = surface->width / (float) surface->glWidth;
    float v_max = surface->height / (float) surface->glHeight;

    glBegin(GL_QUADS);
    glColor3f(1.0f, 1.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(dest->left, dest->top);
    glTexCoord2f(u_max, 0.0f);
    glVertex2f(dest->right, dest->top);
    glTexCoord2f(u_max, v_max);
    glVertex2f(dest->right, dest->bottom);
    glTexCoord2f(0.0f, v_max);
    glVertex2f(dest->left, dest->bottom);
    glEnd();
}

static void update_surface_texture(surface_t* surface, bool transparency, uint16_t transparent_color) {
    DEBUG_PRINT_FUN();

    if (surface->glTexture == 0) {
        surface->glWidth = 0;
        surface->glHeight = 0;

        glGenTextures(1, &surface->glTexture);
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

    uint32_t* buffer = SDL_malloc(surface->glWidth * surface->glHeight * 4);
    uint16_t* data = (uint16_t*) surface->extern_data;

    if (buffer != nullptr && surface->height > 0) {
        for (size_t y = 0; y < surface->height; y++) {
            for (size_t x = 0; x < surface->width; x++) {
                size_t i = (y * surface->width) + x;

                // format is 5515 (red, green, pad, blue)
                uint8_t red = (data[i] & 0xF800) >> 11;
                uint8_t green = (data[i] & 0x07C0) >> 6;
                uint8_t blue = (data[i] & 0x002E);

                if (transparency && data[i] == transparent_color) {
                    buffer[(y * surface->glWidth) + x] = 0x00000000;
                } else {
                    buffer[(y * surface->glWidth) + x] = 0xFF000000 | (red << 3) | ((green << 3) << 8) | ((blue << 3) << 16);
                }
            }
        }

        export_ppm(surface, buffer);

        glBindTexture(GL_TEXTURE_2D, surface->glTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->glWidth, surface->glHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    }

    free(buffer);
    surface->locked = false;
}

static void export_ppm(surface_t* surface, void* buffer) {
    char path[MAX_PATH];
    SDL_snprintf(path, sizeof(path), "surface%u.ppm", surface->glTexture);

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