#include "./blitter.h"

#include <assert.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "render/renderer.h"
#include "render/surfaces.h"
#include "util/debug.h"
#include "util/util.h"

typedef struct [[gnu::aligned(16)]] layout_uniforms {
    float position[2];
    float size[2];
} layout_uniforms_t;

static void update_surface_texture(surface_manager_t* surfaces, surface_t* surface, bool transparency, uint16_t transparent_color);

static void export_ppm(surface_t* surface, void* buffer);

bool blitter__init(blitter_t* self, SDL_GPUDevice* device, surface_manager_t* surfaces, tessellator_t* tessellator) {
    ZERO_INIT_STRUCT(self);

    self->device = device;
    self->surfaces = surfaces;

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
                .r = 0.0f, .g = 0.0f, .b = 1.0f, .a = 1.0f,
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

void blit__blit_to_screen(blitter_t* self, BlitRequest_t const* request, SDL_GPUCommandBuffer* cmd_buffer, SDL_GPURenderPass* render_pass) {
    DEBUG_PRINT_FUN();

    if (request->m_BlitOptions == 0x04) {
        return;
    }

    auto r = renderer__get();

    surface_t* surface = request->m_pSurface;
    DRect_t* dest = request->m_pDestRect;

    if (surface->locked || surface->texture == TEXTURE_ID_INVALID) {
        update_surface_texture(self->surfaces, surface, (request->m_BlitOptions & 0x01) != 0, request->m_TransparentColor);
        return;
    }

    // if (!texture_manager__is_valid(self->surfaces->textures, surface->texture)) {
    //     return;
    // }

    shader_config_t shader_config = {
        .blend = (request->m_BlitOptions & BlitRequestOptions_Transparent) != 0,
    };
    if (!shader__bind(&r->shaders[SHADER_ID__BLIT_2D], shader_config, render_pass)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not bind shader while blitting");
        return;
    }

    layout_uniforms_t layout_uniforms = {
        .position = {
            dest->left,
            dest->top,
        },
        .size = {
            dest->right - dest->left,
            dest->bottom - dest->top,
        }
    };

    SDL_PushGPUVertexUniformData(cmd_buffer, 0, &layout_uniforms, sizeof(layout_uniforms));

    texture_manager__bind_texture(
        self->surfaces->textures,
        0, surface->texture,
        (sampler_config_t) {
            .min_filter = SDL_GPU_FILTER_NEAREST,
            .mag_filter = SDL_GPU_FILTER_NEAREST,
            .wrap_s = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
            .wrap_t = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
        },
        render_pass
    );

    mesh__draw(&self->mesh, render_pass);
    printf("BLIT\n");
}

static void update_surface_texture(surface_manager_t* surfaces, surface_t* surface, bool transparency, uint16_t transparent_color) {
    DEBUG_PRINT_FUN();

    uint32_t* buffer = SDL_malloc(surface->width * surface->height * 4);
    uint16_t* data = surface->extern_data;

    if (surface->texture == TEXTURE_ID_INVALID) {
        surface->texture = texture_manager__create_texture(surfaces->textures);
        if (surface->texture == TEXTURE_ID_INVALID) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create texture for surface");
            return;
        }
    }

    if (buffer != nullptr && surface->height > 0) {
        for (size_t y = 0; y < surface->height; y++) {
            for (size_t x = 0; x < surface->width; x++) {
                size_t i = (y * surface->width) + x;
                uint8_t red = (data[i] & 0xF800) >> 11;
                uint8_t green = (data[i] & 0x07C0) >> 6;
                uint8_t blue = (data[i] & 0x003F) >> 1;

                if (transparency && data[i] == transparent_color) {
                    buffer[(y * surface->width) + x] = 0x00000000;
                } else {
                    buffer[(y * surface->width) + x] = 0xFF000000 | (red << 3) | ((green << 3) << 8) | ((blue << 3) << 16);
                }
            }
        }

        export_ppm(surface, buffer);

        texture_manager__upload_texture_data(surfaces->textures, surface->texture, buffer, surface->width, surface->height);
    }

    SDL_free(buffer);
    surface->locked = false;
}

static void export_ppm(surface_t* surface, void* buffer) {
    char path[MAX_PATH];
    snprintf(path, sizeof(path), "surface%u.ppm", surface->texture);

    FILE* file = fopen(path, "wb");
    if (file == nullptr) {
        return;
    }

    uint8_t* buf = buffer;

    fprintf(file, "P3\n%u %u\n255\n", surface->width, surface->height);
    for (size_t y = 0; y < surface->height; y++) {
        for (size_t x = 0; x < surface->width; x++) {
            size_t i = (y * surface->width * 4) + x * 4;
            fprintf(file, "%hhu %hhu %hhu\n", buf[i], buf[i + 1], buf[i + 2]);
        }
    }

    fclose(file);
}