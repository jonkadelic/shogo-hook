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
            return false;
        }

        desc->dwWidth = w;
        desc->dwHeight = h;
        desc->dwHeight |= DDSD_WIDTH | DDSD_HEIGHT;
    }

    self->width = desc->dwWidth;
    self->height = desc->dwHeight;

    return true;
}

void ddraw_surface__cleanup(ddraw_surface_t* self) {
    // Do nothing
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
    return DDERR_UNSUPPORTED;
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

STDMETHODIMP dsurface_Lock(THIS_ LPRECT,LPDDSURFACEDESC2,DWORD,HANDLE) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
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