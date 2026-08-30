#pragma once

#include "render/shader.h"

typedef enum shader_id {
    SHADER_ID__BLIT_2D,
    NUM_SHADER_IDS,
} shader_id_t;

typedef struct shader_def {
    char const* name;
    char const* shader_srcs[NUM_SHADER_SRC_TYPES];
} shader_def_t;

extern shader_def_t const SHADER_DEFS[];
