#pragma once

#include <SDL3/SDL.h>

#include <lithtech/lithtech.h>

#include "render/blitter.h"
#include "render/shader.h"
#include "render/shared_textures.h"
#include "render/surfaces.h"
#include "render/tessellator.h"

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

void renderer__draw_object(DObject_t const* object);

void renderer__set_camera(DVector_t pos, DRotation_t rotation, float fov_y, float aspect);
HMM_Mat4 renderer__get_view_projection_matrix(void);

surface_manager_t* renderer__get_surfaces(void);
tessellator_t* renderer__get_tessellator(void);
shader_t const* renderer__get_shaders(void);
blitter_t* renderer__get_blitter(void);
shared_texture_manager_t* renderer__get_shared_textures(void);
