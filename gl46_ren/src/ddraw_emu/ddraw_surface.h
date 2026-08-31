#pragma once

#include <ddraw.h>

#include "ddraw_emu/ddraw_iface.h"

typedef struct ddraw_surface {
    IDirectDrawSurface4 base;
    IDirectDrawSurface4Vtbl vtable;
    
    ddraw_iface_t* iface;

    int64_t rc;
    size_t width, height;
} ddraw_surface_t;

bool ddraw_surface__init(ddraw_surface_t* self, ddraw_iface_t* iface, LPDDSURFACEDESC2 desc);
void ddraw_surface__cleanup(ddraw_surface_t* self);
