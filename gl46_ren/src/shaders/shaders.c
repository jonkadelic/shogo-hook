#include "./shaders.h"

shader_def_t const SHADER_DEFS[] = {
    [SHADER_ID__BLIT_2D] = (shader_def_t) {
        .name = "blit_2d",
        .shader_srcs = {
            [SHADER_SRC_TYPE__VERTEX] = (char const[]) {
                #embed "blit_2d.vert"
                , '\0'
            },
            [SHADER_SRC_TYPE__FRAGMENT] = (char const[]) {
                #embed "blit_2d.frag"
                , '\0'
            }
        }
    },
    [SHADER_ID__POLYGRID] = (shader_def_t) {
        .name = "polygrid",
        .shader_srcs = {
            [SHADER_SRC_TYPE__VERTEX] = (char const[]) {
                #embed "polygrid.vert"
                , '\0'
            },
            [SHADER_SRC_TYPE__FRAGMENT] = (char const[]) {
                #embed "polygrid.frag"
                , '\0'
            }
        }
    },
    [SHADER_ID__SCREEN] = (shader_def_t) {
        .name = "screen",
        .shader_srcs = {
            [SHADER_SRC_TYPE__VERTEX] = (char const[]) {
                #embed "screen.vert"
                , '\0'
            },
            [SHADER_SRC_TYPE__FRAGMENT] = (char const[]) {
                #embed "screen.frag"
                , '\0'
            }
        }
    },
    [SHADER_ID__WORLD] = (shader_def_t) {
        .name = "world",
        .shader_srcs = {
            [SHADER_SRC_TYPE__VERTEX] = (char const[]) {
                #embed "world.vert"
                , '\0'
            },
            [SHADER_SRC_TYPE__FRAGMENT] = (char const[]) {
                #embed "world.frag"
                , '\0'
            }
        }
    },
    [SHADER_ID__MODEL] = (shader_def_t) {
        .name = "model",
        .shader_srcs = {
            [SHADER_SRC_TYPE__VERTEX] = (char const[]) {
                #embed "model.vert"
                , '\0'
            },
            [SHADER_SRC_TYPE__FRAGMENT] = (char const[]) {
                #embed "model.frag"
                , '\0'
            }
        }
    },
};
static_assert(sizeof(SHADER_DEFS) / sizeof(shader_def_t) == NUM_SHADER_IDS);