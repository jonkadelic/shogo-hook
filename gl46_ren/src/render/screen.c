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

    if (!pixel_buffer__init(&self->buffer, width, height, COLOR_FORMAT__RGB565)) {
        LOG_ERROR("Failed to init 16-bit screen rect buffer");
        goto err;
    }
    pixel_buffer__clear(&self->buffer, nullptr, 0x0000);

    auto t = renderer__get_tessellator();
    
    tessellator__append_vertices(t,
        4,
        (vertex_t[4]) {
            (vertex_t) {
                .position = { -1.0f, -1.0f, 0.0f },
                .color = { 1.0f, 1.0f, 1.0f, 1.0f },
                .uv = { 0.0f, 1.0f },
            },
            (vertex_t) {
                .position = { 1.0f, -1.0f, 0.0f },
                .color = { 1.0f, 1.0f, 1.0f, 1.0f },
                .uv = { 1.0f, 1.0f },
            },
            (vertex_t) {
                .position = { -1.0f, 1.0f, 0.0f },
                .color = { 1.0f, 1.0f, 1.0f, 1.0f },
                .uv = { 0.0f, 0.0f },
            },
            (vertex_t) {
                .position = { 1.0f, 1.0f, 0.0f },
                .color = { 1.0f, 1.0f, 1.0f, 1.0f },
                .uv = { 1.0f, 0.0f },
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

    return true;

err:
    screen__cleanup(self);
    return false;
}

void screen__cleanup(screen_t* self) {
    pixel_buffer__cleanup(&self->buffer);
    texture__cleanup(&self->texture);
    mesh__cleanup(&self->mesh);    
}

void* screen__lock(screen_t* self) {
    self->locked = true;
    return self->buffer.data;
}

void screen__draw(screen_t* self) {
    if (self->locked) {
        texture__upload_rect_buffer(&self->texture, &self->buffer);
        self->locked = false;
    }

    auto r = renderer__get_instance();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    shader_t* shader = &r->shaders[SHADER_ID__SCREEN];
    shader__bind(shader);
    shader__set_uniform_texture(shader, "u_texture", &self->texture);
    
    mesh__draw(&self->mesh);
}

void screen__clear(screen_t* self) {
    pixel_buffer__clear(&self->buffer, nullptr, 0x0000);
    texture__upload_rect_buffer(&self->texture, &self->buffer);
}
