#pragma once

#include "ddraw_emu/ddraw_surface.h"

typedef struct ddraw_backbuffer {
    ddraw_surface_t base;
    renderer_t* renderer;
} ddraw_backbuffer_t;

bool ddraw_backbuffer__init(ddraw_backbuffer_t* self, ddraw_iface_t* iface);
void ddraw_backbuffer__cleanup(ddraw_backbuffer_t* self);
