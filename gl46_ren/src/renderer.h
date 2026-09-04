#pragma once

#include <SDL3/SDL.h>

#include <lithtech/lithtech.h>

#include "ddraw_emu/ddraw_backbuffer.h"
#include "ddraw_emu/ddraw_iface.h"
#include "render/blitter.h"
#include "render/object/objects.h"
#include "render/screen.h"
#include "render/shader.h"
#include "render/shared_textures.h"
#include "render/rsurfaces.h"
#include "render/tessellator.h"
#include "render/world/world_renderer.h"
#include "shaders/shaders.h"

typedef struct renderer {
    bool in_3d;
    bool in_2d;

    void* hwnd;
    SDL_Window* window;
    SDL_GLContext gl_context;
    uint64_t last_swap;
    float yfov_mod;

    rsurface_manager_t rsurfaces;
    tessellator_t tessellator;
    tessellator_t model_tessellator;
    shader_t shaders[NUM_SHADER_IDS];
    blitter_t blitter;
    object_manager_t objects;
    shared_texture_manager_t shared_textures;
    pixel_buffer_t backbuffer;
    screen_t screen;
    RenderContext_t* render_context;
    world_renderer_t* world;
    
    struct {
        DVector_t pos;
        DRotation_t rotation;
        float fov_y;
        float aspect;
    } camera;

    struct {
        ddraw_iface_t iface;
        ddraw_backbuffer_t backbuffer;
    } ddraw;
} renderer_t;

renderer_t* renderer__get_instance(void);

bool renderer__init(RMode_t const* rmode, void* hwnd);
void renderer__cleanup(void);

void renderer__reset(void);

bool renderer__start_3d(void);
bool renderer__end_3d(void);
bool renderer__is_in_3d(void);

bool renderer__start_2d(void);
bool renderer__end_2d(void);
bool renderer__is_in_2d(void);

void renderer__swap_buffers(void);

void renderer__draw_object(SceneDesc_t const* scene_desc, DObject_t const* object);

void renderer__set_camera(DVector_t pos, DRotation_t rotation, float fov_y, float aspect);
HMM_Mat4 renderer__get_view_projection_matrix(void);

rsurface_manager_t* renderer__get_rsurfaces(void);
tessellator_t* renderer__get_tessellator(void);
tessellator_t* renderer__get_model_tessellator(void);
shader_t const* renderer__get_shaders(void);
blitter_t* renderer__get_blitter(void);
shared_texture_manager_t* renderer__get_shared_textures(void);
