#include "./ddraw_surface.h"

#include <SDL3/SDL.h>

#include "logger.h"
#include "util/util.h"
#include "renderer.h"

#undef INTERFACE
#define INTERFACE IDirectDrawSurface4
/*** IUnknown methods ***/
STDMETHODIMP dsurface_QueryInterface (THIS_ REFIID riid, LPVOID FAR * ppvObj) PURE;
STDMETHODIMP_(ULONG) dsurface_AddRef (THIS)  PURE;
STDMETHODIMP_(ULONG) dsurface_Release (THIS) PURE;
/*** IDirectDrawSurface methods ***/
STDMETHODIMP dsurface_AddAttachedSurface(THIS_ LPDIRECTDRAWSURFACE4) PURE;
STDMETHODIMP dsurface_AddOverlayDirtyRect(THIS_ LPRECT) PURE;
STDMETHODIMP dsurface_Blt(THIS_ LPRECT,LPDIRECTDRAWSURFACE4, LPRECT,DWORD, LPDDBLTFX) PURE;
STDMETHODIMP dsurface_BltBatch(THIS_ LPDDBLTBATCH, DWORD, DWORD ) PURE;
STDMETHODIMP dsurface_BltFast(THIS_ DWORD,DWORD,LPDIRECTDRAWSURFACE4, LPRECT,DWORD) PURE;
STDMETHODIMP dsurface_DeleteAttachedSurface(THIS_ DWORD,LPDIRECTDRAWSURFACE4) PURE;
STDMETHODIMP dsurface_EnumAttachedSurfaces(THIS_ LPVOID,LPDDENUMSURFACESCALLBACK2) PURE;
STDMETHODIMP dsurface_EnumOverlayZOrders(THIS_ DWORD,LPVOID,LPDDENUMSURFACESCALLBACK2) PURE;
STDMETHODIMP dsurface_Flip(THIS_ LPDIRECTDRAWSURFACE4, DWORD) PURE;
STDMETHODIMP dsurface_GetAttachedSurface(THIS_ LPDDSCAPS2, LPDIRECTDRAWSURFACE4 FAR *) PURE;
STDMETHODIMP dsurface_GetBltStatus(THIS_ DWORD) PURE;
STDMETHODIMP dsurface_GetCaps(THIS_ LPDDSCAPS2) PURE;
STDMETHODIMP dsurface_GetClipper(THIS_ LPDIRECTDRAWCLIPPER FAR*) PURE;
STDMETHODIMP dsurface_GetColorKey(THIS_ DWORD, LPDDCOLORKEY) PURE;
STDMETHODIMP dsurface_GetDC(THIS_ HDC FAR *) PURE;
STDMETHODIMP dsurface_GetFlipStatus(THIS_ DWORD) PURE;
STDMETHODIMP dsurface_GetOverlayPosition(THIS_ LPLONG, LPLONG ) PURE;
STDMETHODIMP dsurface_GetPalette(THIS_ LPDIRECTDRAWPALETTE FAR*) PURE;
STDMETHODIMP dsurface_GetPixelFormat(THIS_ LPDDPIXELFORMAT) PURE;
STDMETHODIMP dsurface_GetSurfaceDesc(THIS_ LPDDSURFACEDESC2) PURE;
STDMETHODIMP dsurface_Initialize(THIS_ LPDIRECTDRAW, LPDDSURFACEDESC2) PURE;
STDMETHODIMP dsurface_IsLost(THIS) PURE;
STDMETHODIMP dsurface_Lock(THIS_ LPRECT,LPDDSURFACEDESC2,DWORD,HANDLE) PURE;
STDMETHODIMP dsurface_ReleaseDC(THIS_ HDC) PURE;
STDMETHODIMP dsurface_Restore(THIS) PURE;
STDMETHODIMP dsurface_SetClipper(THIS_ LPDIRECTDRAWCLIPPER) PURE;
STDMETHODIMP dsurface_SetColorKey(THIS_ DWORD, LPDDCOLORKEY) PURE;
STDMETHODIMP dsurface_SetOverlayPosition(THIS_ LONG, LONG ) PURE;
STDMETHODIMP dsurface_SetPalette(THIS_ LPDIRECTDRAWPALETTE) PURE;
STDMETHODIMP dsurface_Unlock(THIS_ LPRECT) PURE;
STDMETHODIMP dsurface_UpdateOverlay(THIS_ LPRECT, LPDIRECTDRAWSURFACE4,LPRECT,DWORD, LPDDOVERLAYFX) PURE;
STDMETHODIMP dsurface_UpdateOverlayDisplay(THIS_ DWORD) PURE;
STDMETHODIMP dsurface_UpdateOverlayZOrder(THIS_ DWORD, LPDIRECTDRAWSURFACE4) PURE;
/*** Added in the v2 interface ***/
STDMETHODIMP dsurface_GetDDInterface(THIS_ LPVOID FAR *) PURE;
STDMETHODIMP dsurface_PageLock(THIS_ DWORD) PURE;
STDMETHODIMP dsurface_PageUnlock(THIS_ DWORD) PURE;
/*** Added in the v3 interface ***/
STDMETHODIMP dsurface_SetSurfaceDesc(THIS_ LPDDSURFACEDESC2, DWORD) PURE;
/*** Added in the v4 interface ***/
STDMETHODIMP dsurface_SetPrivateData(THIS_ REFGUID, LPVOID, DWORD, DWORD) PURE;
STDMETHODIMP dsurface_GetPrivateData(THIS_ REFGUID, LPVOID, LPDWORD) PURE;
STDMETHODIMP dsurface_FreePrivateData(THIS_ REFGUID) PURE;
STDMETHODIMP dsurface_GetUniquenessValue(THIS_ LPDWORD) PURE;
STDMETHODIMP dsurface_ChangeUniquenessValue(THIS) PURE;

bool ddraw_surface__init(ddraw_surface_t* self, ddraw_iface_t* iface, LPDDSURFACEDESC2 desc) {
    OBJECT_ZERO_INIT(self);

    self->base.lpVtbl = &self->vtable;
    self->vtable = (IDirectDrawSurface4Vtbl) {
        .QueryInterface = dsurface_QueryInterface,
        .AddRef = dsurface_AddRef,
        .Release = dsurface_Release,
        .AddAttachedSurface = dsurface_AddAttachedSurface,
        .AddOverlayDirtyRect = dsurface_AddOverlayDirtyRect,
        .Blt = dsurface_Blt,
        .BltBatch = dsurface_BltBatch,
        .BltFast = dsurface_BltFast,
        .DeleteAttachedSurface = dsurface_DeleteAttachedSurface,
        .EnumAttachedSurfaces = dsurface_EnumAttachedSurfaces,
        .EnumOverlayZOrders = dsurface_EnumOverlayZOrders,
        .Flip = dsurface_Flip,
        .GetAttachedSurface = dsurface_GetAttachedSurface,
        .GetBltStatus = dsurface_GetBltStatus,
        .GetCaps = dsurface_GetCaps,
        .GetClipper = dsurface_GetClipper,
        .GetColorKey = dsurface_GetColorKey,
        .GetDC = dsurface_GetDC,
        .GetFlipStatus = dsurface_GetFlipStatus,
        .GetOverlayPosition = dsurface_GetOverlayPosition,
        .GetPalette = dsurface_GetPalette,
        .GetPixelFormat = dsurface_GetPixelFormat,
        .GetSurfaceDesc = dsurface_GetSurfaceDesc,
        .Initialize = dsurface_Initialize,
        .IsLost = dsurface_IsLost,
        .Lock = dsurface_Lock,
        .ReleaseDC = dsurface_ReleaseDC,
        .Restore = dsurface_Restore,
        .SetClipper = dsurface_SetClipper,
        .SetColorKey = dsurface_SetColorKey,
        .SetOverlayPosition = dsurface_SetOverlayPosition,
        .SetPalette = dsurface_SetPalette,
        .Unlock = dsurface_Unlock,
        .UpdateOverlay = dsurface_UpdateOverlay,
        .UpdateOverlayDisplay = dsurface_UpdateOverlayDisplay,
        .UpdateOverlayZOrder = dsurface_UpdateOverlayZOrder,
        .GetDDInterface = dsurface_GetDDInterface,
        .PageLock = dsurface_PageLock,
        .PageUnlock = dsurface_PageUnlock,
        .SetSurfaceDesc = dsurface_SetSurfaceDesc,
        .SetPrivateData = dsurface_SetPrivateData,
        .GetPrivateData = dsurface_GetPrivateData,
        .FreePrivateData = dsurface_FreePrivateData,
        .GetUniquenessValue = dsurface_GetUniquenessValue,
        .ChangeUniquenessValue = dsurface_ChangeUniquenessValue,
    };
    self->iface = iface;
    
    if ((desc->dwFlags & (DDSD_WIDTH | DDSD_HEIGHT)) == 0) {
        int w, h;
        if (!SDL_GetWindowSize(iface->renderer->window, &w, &h)) {
            LOG_ERROR("Failed to get window size");
            goto err;
        }

        desc->dwWidth = w;
        desc->dwHeight = h;
        desc->dwHeight |= DDSD_WIDTH | DDSD_HEIGHT;
    }
    if ((desc->dwFlags & DDSD_PIXELFORMAT) == 0) {
        desc->ddpfPixelFormat = (typeof(desc->ddpfPixelFormat)) { 
            .dwFlags = DDPF_RGB,
            .dwRGBBitCount = 16, // assume 16-bit buffer as Shogo will likely do the same
        };
        desc->dwFlags |= DDSD_PIXELFORMAT;
    }

    self->owns_buffer = true;
    self->buffer = SDL_malloc(sizeof(pixel_buffer_t));
    if (self->buffer == nullptr) {
        LOG_ERROR("Failed to alloc buffer for surface");
        goto err;
    }

    color_format_t format;
    if (desc->ddpfPixelFormat.dwRGBBitCount == 32) {
        format = COLOR_FORMAT__RGBA32;
    } else if (desc->ddpfPixelFormat.dwRGBBitCount == 16) {
        format = COLOR_FORMAT__RGB565;
    } else {
        LOG_ERROR("Unsupported pixel format");
        goto err;
    }

    if (!pixel_buffer__init(self->buffer, desc->dwWidth, desc->dwHeight, format)) {
        LOG_ERROR("Failed to init surface buffer");
        goto err;
    }
    self->format = desc->ddpfPixelFormat;

    return true;

err:
    ddraw_surface__cleanup(self);
    return false;
}

void ddraw_surface__cleanup(ddraw_surface_t* self) {
    if (self->owns_buffer && self->buffer != nullptr) {
        pixel_buffer__cleanup(self->buffer);

        SDL_free(self->buffer);
    }

    self->buffer = nullptr;
}

#undef INTERFACE
#define INTERFACE IDirectDrawSurface4
/*** IUnknown methods ***/
STDMETHODIMP dsurface_QueryInterface (THIS_ REFIID riid, LPVOID FAR * ppvObj) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP_(ULONG) dsurface_AddRef (THIS)  PURE {
    LOG_FUNC();
    
    auto self = (ddraw_surface_t*) This;
    return ++self->rc;
}

STDMETHODIMP_(ULONG) dsurface_Release (THIS) PURE {
    LOG_FUNC();

    auto self = (ddraw_surface_t*) This;
    auto new_rc = --self->rc;

    if (new_rc <= 0) {
        ddraw_surface__cleanup(self);
        SDL_free(self);
    }

    return new_rc;
}

/*** IDirectDrawSurface methods ***/
STDMETHODIMP dsurface_AddAttachedSurface(THIS_ LPDIRECTDRAWSURFACE4) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP dsurface_AddOverlayDirtyRect(THIS_ LPRECT) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP dsurface_Blt(THIS_ LPRECT dst_rect, LPDIRECTDRAWSURFACE4 src_surface, LPRECT src_rect, DWORD flags, LPDDBLTFX blt_fx) PURE {
    LOG_FUNC();

    auto self = (ddraw_surface_t*) This;

    static DWORD supported_flags =
        DDBLT_COLORFILL;

    if ((flags & ~supported_flags) > 0) {
        return DDERR_UNSUPPORTED;
    }

    rect_t r_dst_rect;
    rect_t* p_dst_rect = nullptr;
    rect_t r_src_rect;
    rect_t* p_src_rect = nullptr;
    
    if (dst_rect != nullptr) {
        p_dst_rect = &r_dst_rect;
        p_dst_rect->x0 = dst_rect->left;
        p_dst_rect->y0 = dst_rect->top;
        p_dst_rect->x1 = dst_rect->right;
        p_dst_rect->y1 = dst_rect->bottom;
    }
    if (src_rect != nullptr) {
        p_src_rect = &r_src_rect;
        p_src_rect->x0 = src_rect->left;
        p_src_rect->y0 = src_rect->top;
        p_src_rect->x1 = src_rect->right;
        p_src_rect->y1 = src_rect->bottom;
    }

    if ((flags & DDBLT_COLORFILL) > 0) {
        pixel_buffer__clear(self->buffer, p_dst_rect, blt_fx->dwFillColor);
    }

    if (src_surface != nullptr) {
        ddraw_surface_t* surface = (ddraw_surface_t*) src_surface;
        if (!pixel_buffer__copy(self->buffer, surface->buffer, p_src_rect, p_dst_rect)) {
            return DDERR_INVALIDRECT;
        }
    }

    return DD_OK;
}

STDMETHODIMP dsurface_BltBatch(THIS_ LPDDBLTBATCH, DWORD, DWORD ) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP dsurface_BltFast(THIS_ DWORD,DWORD,LPDIRECTDRAWSURFACE4, LPRECT,DWORD) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP dsurface_DeleteAttachedSurface(THIS_ DWORD,LPDIRECTDRAWSURFACE4) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP dsurface_EnumAttachedSurfaces(THIS_ LPVOID,LPDDENUMSURFACESCALLBACK2) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP dsurface_EnumOverlayZOrders(THIS_ DWORD,LPVOID,LPDDENUMSURFACESCALLBACK2) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP dsurface_Flip(THIS_ LPDIRECTDRAWSURFACE4, DWORD) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP dsurface_GetAttachedSurface(THIS_ LPDDSCAPS2, LPDIRECTDRAWSURFACE4 FAR *) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP dsurface_GetBltStatus(THIS_ DWORD) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP dsurface_GetCaps(THIS_ LPDDSCAPS2) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP dsurface_GetClipper(THIS_ LPDIRECTDRAWCLIPPER FAR*) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP dsurface_GetColorKey(THIS_ DWORD, LPDDCOLORKEY) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP dsurface_GetDC(THIS_ HDC FAR *) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP dsurface_GetFlipStatus(THIS_ DWORD) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP dsurface_GetOverlayPosition(THIS_ LPLONG, LPLONG ) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP dsurface_GetPalette(THIS_ LPDIRECTDRAWPALETTE FAR*) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP dsurface_GetPixelFormat(THIS_ LPDDPIXELFORMAT) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP dsurface_GetSurfaceDesc(THIS_ LPDDSURFACEDESC2) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP dsurface_Initialize(THIS_ LPDIRECTDRAW, LPDDSURFACEDESC2) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP dsurface_IsLost(THIS) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP dsurface_Lock(THIS_ LPRECT dst_rect, LPDDSURFACEDESC2 desc, DWORD flags, HANDLE unused) PURE {
    LOG_FUNC();

    auto self = (ddraw_surface_t*) This;

    if (desc == nullptr) {
        return DDERR_INVALIDPARAMS;
    }

    int left = 0, top = 0;
    if (dst_rect != nullptr) {
        if (dst_rect->left < 0 || dst_rect->top < 0 ||
            dst_rect->right > self->buffer->width || dst_rect->bottom > self->buffer->height ||
            dst_rect->left >= dst_rect->right || dst_rect->top >= dst_rect->bottom
        ) {
            return DDERR_INVALIDRECT;
        }

        left = dst_rect->left;
        top = dst_rect->top;
    }

    auto bpp = color_format__get_bpp(self->buffer->format);

    uint32_t pitch = self->buffer->width * bpp;

    desc->dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PITCH | DDSD_LPSURFACE | DDSD_PIXELFORMAT;
    desc->dwWidth = self->buffer->width;
    desc->dwHeight = self->buffer->height;
    desc->lPitch = pitch;
    desc->lpSurface = self->buffer->data + (top * pitch) + (left * bpp);
    desc->ddpfPixelFormat = self->format;
    desc->ddpfPixelFormat.dwSize = sizeof(desc->ddpfPixelFormat);

    return DD_OK;
}

STDMETHODIMP dsurface_ReleaseDC(THIS_ HDC) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP dsurface_Restore(THIS) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP dsurface_SetClipper(THIS_ LPDIRECTDRAWCLIPPER) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP dsurface_SetColorKey(THIS_ DWORD, LPDDCOLORKEY) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP dsurface_SetOverlayPosition(THIS_ LONG, LONG ) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP dsurface_SetPalette(THIS_ LPDIRECTDRAWPALETTE) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP dsurface_Unlock(THIS_ LPRECT) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP dsurface_UpdateOverlay(THIS_ LPRECT, LPDIRECTDRAWSURFACE4,LPRECT,DWORD, LPDDOVERLAYFX) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP dsurface_UpdateOverlayDisplay(THIS_ DWORD) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP dsurface_UpdateOverlayZOrder(THIS_ DWORD, LPDIRECTDRAWSURFACE4) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}
/*** Added in the v2 interface ***/
STDMETHODIMP dsurface_GetDDInterface(THIS_ LPVOID FAR *) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP dsurface_PageLock(THIS_ DWORD) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP dsurface_PageUnlock(THIS_ DWORD) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}
/*** Added in the v3 interface ***/
STDMETHODIMP dsurface_SetSurfaceDesc(THIS_ LPDDSURFACEDESC2, DWORD) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}
/*** Added in the v4 interface ***/
STDMETHODIMP dsurface_SetPrivateData(THIS_ REFGUID, LPVOID, DWORD, DWORD) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP dsurface_GetPrivateData(THIS_ REFGUID, LPVOID, LPDWORD) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP dsurface_FreePrivateData(THIS_ REFGUID) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP dsurface_GetUniquenessValue(THIS_ LPDWORD) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP dsurface_ChangeUniquenessValue(THIS) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}