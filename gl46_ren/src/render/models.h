#pragma once

#include <stdint.h>

#include "render/model_renderer.h"

typedef struct model_manager_entry {
    uint64_t path_hash;
    model_renderer_t renderer;
} model_manager_entry_t;

typedef struct model_manager {
    size_t models_len, models_capacity;
    model_manager_entry_t** models;
} model_manager_t;

bool model_manager__init(model_manager_t* self);
void model_manager__cleanup(model_manager_t* self);

model_renderer_t* model_manager__get_renderer(model_manager_t* self, ModelData_t const* model_data);
model_renderer_t* model_manager__get_renderer_by_filename(model_manager_t* self, char const* filename);
