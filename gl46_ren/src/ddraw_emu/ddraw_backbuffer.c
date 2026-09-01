#include "./ddraw_backbuffer.h"

#include "logger.h"
#include "util/util.h"
#include "renderer.h"

#undef INTERFACE
#define INTERFACE IDirectDrawSurface4
STDMETHODIMP_(ULONG) dbackbuffer_AddRef (THIS)  PURE;
STDMETHODIMP_(ULONG) dbackbuffer_Release (THIS) PURE;

bool ddraw_backbuffer__init(ddraw_backbuffer_t* self, ddraw_iface_t* iface) {
    OBJECT_ZERO_INIT(self);

    int w, h;
    if (!SDL_GetWindowSize(iface->renderer->window, &w, &h)) {
        LOG_ERROR("Failed to get window size");
        return false;
    }

    DDSURFACEDESC2 desc = {
        .dwSize = sizeof(DDSURFACEDESC2),
        .dwFlags =
            DDSD_WIDTH |
            DDSD_HEIGHT
        ,
        .dwWidth = w,
        .dwHeight = h,
    };

    if (!ddraw_surface__init((ddraw_surface_t*) self, iface, &desc)) {
        LOG_ERROR("Failed to init backing surface");
        return false;
    }

    // Overrides
    self->base.vtable.AddRef = dbackbuffer_AddRef;
    self->base.vtable.Release = dbackbuffer_Release;

    pixel_buffer__cleanup(self->base.buffer);
    SDL_free(self->base.buffer);

    self->base.owns_buffer = false;
    self->base.buffer = &iface->renderer->screen.buffer;

    return true;
}

void ddraw_backbuffer__cleanup(ddraw_backbuffer_t* self) {
    ddraw_surface__cleanup(&self->base);
}

#undef INTERFACE
#define INTERFACE IDirectDrawSurface4
STDMETHODIMP_(ULONG) dbackbuffer_AddRef (THIS)  PURE {
    LOG_FUNC();

    return DDERR_UNSUPPORTED;
}

STDMETHODIMP_(ULONG) dbackbuffer_Release (THIS) PURE {
    LOG_FUNC();

    return DDERR_UNSUPPORTED;
}
