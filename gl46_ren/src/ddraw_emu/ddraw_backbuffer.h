#pragma once

#include "ddraw_emu/ddraw_surface.h"

typedef struct ddraw_backbuffer {
    ddraw_surface_t base;
    pixel_buffer_t* backbuffer;
} ddraw_backbuffer_t;

bool ddraw_backbuffer__init(ddraw_backbuffer_t* self, ddraw_iface_t* iface, pixel_buffer_t* backbuffer);
void ddraw_backbuffer__cleanup(ddraw_backbuffer_t* self);
