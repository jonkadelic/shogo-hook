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
    }
};
static_assert(sizeof(SHADER_DEFS) / sizeof(shader_def_t) == NUM_SHADER_IDS);