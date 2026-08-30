#pragma once

#include <SDL3/SDL.h>

#include <lithtech/lithtech.h>

#include "render/blitter.h"
#include "render/shader.h"
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

surface_manager_t* renderer__get_surfaces(void);
tessellator_t* renderer__get_tessellator(void);
shader_t const* renderer__get_shaders(void);
blitter_t* renderer__get_blitter(void);