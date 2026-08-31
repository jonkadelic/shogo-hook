#include "./screen.h"

#include "logger.h"
#include "renderer.h"
#include "util/util.h"

bool screen__init(screen_t* self, size_t width, size_t height) {
    OBJECT_ZERO_INIT(self);

    if (!mesh__init(&self->mesh)) {
        LOG_ERROR("Failed to init screen mesh");
        goto err;
    }

    if (!texture__init(&self->texture, "Screen Texture")) {
        LOG_ERROR("Failed to init screen texture");
        goto err;
    }

    if (!rect_buffer__init(&self->buffer, width, height, 2)) {
        LOG_ERROR("Failed to init 24-bit screen rect buffer");
        goto err;
    }
    rect_buffer__clear(&self->buffer, nullptr, 0x00000000);

    if (!rect_buffer__init(&self->buffer_32, width, height, 4)) {
        LOG_ERROR("Failed to init 32-bit screen rect buffer");
        goto err;
    }
    rect_buffer__clear(&self->buffer_32, nullptr, 0x00000000);

    auto t = renderer__get_tessellator();
    
    tessellator__append_vertices(t,
        4,
        (vertex_t[4]) {
            (vertex_t) {
                .position = { 0.0f, 0.0f, 0.0f },
                .color = { 1.0f, 1.0f, 1.0f, 1.0f },
                .uv = { 0.0f, 0.0f },
            },
            (vertex_t) {
                .position = { 1.0f, 0.0f, 0.0f },
                .color = { 1.0f, 1.0f, 1.0f, 1.0f },
                .uv = { 1.0f, 0.0f },
            },
            (vertex_t) {
                .position = { 0.0f, 1.0f, 0.0f },
                .color = { 1.0f, 1.0f, 1.0f, 1.0f },
                .uv = { 0.0f, 1.0f },
            },
            (vertex_t) {
                .position = { 1.0f, 1.0f, 0.0f },
                .color = { 1.0f, 1.0f, 1.0f, 1.0f },
                .uv = { 1.0f, 1.0f },
            },
        }
    );
    tessellator__append_indices(t,
        6,
        (index_t[6]) {
            0, 2, 1,
            1, 2, 3
        }
    );
    tessellator__upload_and_reset(t, &self->mesh);

    self->proj_matrix = HMM_Orthographic_RH_NO(0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f);

    return true;

err:
    screen__cleanup(self);
    return false;
}

void screen__cleanup(screen_t* self) {
    rect_buffer__cleanup(&self->buffer_32);
    rect_buffer__cleanup(&self->buffer);
    texture__cleanup(&self->texture);
    mesh__cleanup(&self->mesh);    
}

void* screen__lock(screen_t* self) {
    self->locked = true;
    return self->buffer.data;
}

void screen__draw(screen_t* self) {
    if (self->locked) {
        uint8_t* buffer_32 = (uint8_t*) self->buffer_32.data;
        for (size_t i = 0; i < self->buffer.width * self->buffer.height; i++) {
            size_t bi = i * self->buffer_32.bpp;
            uint16_t val = ((uint16_t*) self->buffer.data)[i];

            if (val > 0x0000) {
                size_t a = 0;
            }

            uint16_t r5 = (val & 0xF800) >> 11;
            uint16_t g6 = (val & 0x07E0) >> 5;
            uint16_t b5 = (val & 0x001F);

            buffer_32[bi + 0] = (r5 << 3) | (r5 >> 2); // r
            buffer_32[bi + 1] = (g6 << 2) | (g6 >> 4); // g
            buffer_32[bi + 2] = (b5 << 3) | (b5 >> 2); // b
            buffer_32[bi + 3] = val != 0x0000 ? 0xFF : 0x00;
        }

        texture__upload_rect_buffer(&self->texture, &self->buffer_32);
        self->locked = false;
    }

    auto r = renderer__get_instance();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    shader_t* shader = &r->shaders[SHADER_ID__BLIT_2D];
    shader__bind(shader);
    shader__set_uniform_texture(&r->shaders[SHADER_ID__BLIT_2D], "u_texture", &self->texture);

    auto model_matrix = HMM_M4D(1.0f);
    
    shader__set_uniform_mat4f(shader, "u_projection", &self->proj_matrix);
    shader__set_uniform_mat4f(shader, "u_model", &model_matrix);

    mesh__draw(&self->mesh);
}

void screen__clear(screen_t* self) {
    rect_buffer__clear(&self->buffer, nullptr, 0x00000000);
    texture__upload_rect_buffer(&self->texture, &self->buffer_32);
}