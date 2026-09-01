#pragma once

#include <ddraw.h>

#include "ddraw_emu/ddraw_iface.h"
#include "render/pixel_buffer.h"

typedef struct ddraw_surface {
    IDirectDrawSurface4 base;
    IDirectDrawSurface4Vtbl vtable;
    
    ddraw_iface_t* iface;

    int64_t rc;
    DDPIXELFORMAT format;
    bool owns_buffer;
    pixel_buffer_t* buffer; // pointer so ddraw_backbuffer can use the real backbuffer instead
} ddraw_surface_t;

bool ddraw_surface__init(ddraw_surface_t* self, ddraw_iface_t* iface, LPDDSURFACEDESC2 desc);
void ddraw_surface__cleanup(ddraw_surface_t* self);
