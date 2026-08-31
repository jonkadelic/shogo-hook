#pragma once

#include <ddraw.h>
#include <stdint.h>

typedef struct renderer renderer_t;

typedef struct ddraw_iface {
    IDirectDraw4 base;
    IDirectDraw4Vtbl vtable;

    renderer_t* renderer;

    int64_t rc;
} ddraw_iface_t;

bool ddraw_iface__init(ddraw_iface_t* self, renderer_t* renderer);
void ddraw_iface__cleanup(ddraw_iface_t* self);
