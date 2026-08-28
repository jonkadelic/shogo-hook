#include "./mesh.h"

#include "util/util.h"

bool mesh__init(mesh_t* self, SDL_GPUDevice* device) {
    ZERO_INIT_STRUCT(self);

    self->device = device;

    return true;
}

void mesh__cleanup(mesh_t* self) {
    SDL_ReleaseGPUBuffer(self->device, self->indices);
    self->indices = nullptr;

    SDL_ReleaseGPUBuffer(self->device, self->vertices);
    self->vertices = nullptr;

    self->device = nullptr;
}

void mesh__draw(mesh_t* self, SDL_GPURenderPass* render_pass) {
    if (self->vertices == nullptr || self->indices == nullptr) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Tried to draw uninitialized mesh");
        return;
    }

    SDL_GPUBufferBinding vertices_binding = {
        .buffer = self->vertices,
        .offset = 0,
    };
    SDL_BindGPUVertexBuffers(render_pass, 0, &vertices_binding, 1);

    SDL_GPUBufferBinding indices_binding = {
        .buffer = self->indices,
        .offset = 0,
    };
    SDL_BindGPUIndexBuffer(render_pass, &indices_binding, SDL_GPU_INDEXELEMENTSIZE_16BIT);

    SDL_DrawGPUIndexedPrimitives(
        render_pass,
        self->indices_len,
        1,
        0,
        0,
        0
    );
}
