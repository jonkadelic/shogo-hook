#include "./ddraw_iface.h"

#include <SDL3/SDL.h>

#include "ddraw_emu/ddraw_surface.h"
#include "logger.h"
#include "util/util.h"
#include "renderer.h"

#undef INTERFACE
#define INTERFACE IDirectDraw4
/*** IUnknown methods ***/
STDMETHODIMP ddraw_QueryInterface (THIS_ REFIID riid, LPVOID FAR * ppvObj) PURE;
STDMETHODIMP_(ULONG) ddraw_AddRef (THIS)  PURE;
STDMETHODIMP_(ULONG) ddraw_Release (THIS) PURE;
/*** IDirectDraw methods ***/
STDMETHODIMP ddraw_Compact(THIS) PURE;
STDMETHODIMP ddraw_CreateClipper(THIS_ DWORD, LPDIRECTDRAWCLIPPER FAR*, IUnknown FAR * ) PURE;
STDMETHODIMP ddraw_CreatePalette(THIS_ DWORD, LPPALETTEENTRY, LPDIRECTDRAWPALETTE FAR*, IUnknown FAR * ) PURE;
STDMETHODIMP ddraw_CreateSurface(THIS_  LPDDSURFACEDESC2 desc, LPDIRECTDRAWSURFACE4 FAR * out, IUnknown FAR * reserved) PURE;
STDMETHODIMP ddraw_DuplicateSurface( THIS_ LPDIRECTDRAWSURFACE4, LPDIRECTDRAWSURFACE4 FAR * ) PURE;
STDMETHODIMP ddraw_EnumDisplayModes( THIS_ DWORD, LPDDSURFACEDESC2, LPVOID, LPDDENUMMODESCALLBACK2 ) PURE;
STDMETHODIMP ddraw_EnumSurfaces(THIS_ DWORD, LPDDSURFACEDESC2, LPVOID,LPDDENUMSURFACESCALLBACK2 ) PURE;
STDMETHODIMP ddraw_FlipToGDISurface(THIS) PURE;
STDMETHODIMP ddraw_GetCaps( THIS_ LPDDCAPS, LPDDCAPS) PURE;
STDMETHODIMP ddraw_GetDisplayMode( THIS_ LPDDSURFACEDESC2 out) PURE;
STDMETHODIMP ddraw_GetFourCCCodes(THIS_  LPDWORD, LPDWORD ) PURE;
STDMETHODIMP ddraw_GetGDISurface(THIS_ LPDIRECTDRAWSURFACE4 FAR *) PURE;
STDMETHODIMP ddraw_GetMonitorFrequency(THIS_ LPDWORD) PURE;
STDMETHODIMP ddraw_GetScanLine(THIS_ LPDWORD) PURE;
STDMETHODIMP ddraw_GetVerticalBlankStatus(THIS_ LPBOOL ) PURE;
STDMETHODIMP ddraw_Initialize(THIS_ GUID FAR *) PURE;
STDMETHODIMP ddraw_RestoreDisplayMode(THIS) PURE;
STDMETHODIMP ddraw_SetCooperativeLevel(THIS_ HWND, DWORD) PURE;
STDMETHODIMP ddraw_SetDisplayMode(THIS_ DWORD, DWORD,DWORD, DWORD, DWORD) PURE ;
STDMETHODIMP ddraw_WaitForVerticalBlank(THIS_ DWORD, HANDLE ) PURE;
/*** Added in the v2 interface ***/
STDMETHODIMP ddraw_GetAvailableVidMem(THIS_ LPDDSCAPS2, LPDWORD, LPDWORD) PURE;
/*** Added in the V4 Interface ***/
STDMETHODIMP ddraw_GetSurfaceFromDC (THIS_ HDC, LPDIRECTDRAWSURFACE4 *) PURE;
STDMETHODIMP ddraw_RestoreAllSurfaces(THIS) PURE;
STDMETHODIMP ddraw_TestCooperativeLevel(THIS) PURE;
STDMETHODIMP ddraw_GetDeviceIdentifier(THIS_ LPDDDEVICEIDENTIFIER, DWORD ) PURE;

bool ddraw_iface__init(ddraw_iface_t* self, renderer_t* renderer) {
    OBJECT_ZERO_INIT(self);

    self->base.lpVtbl = &self->vtable;
    self->vtable = (IDirectDraw4Vtbl) {
        .QueryInterface = ddraw_QueryInterface,
        .AddRef = ddraw_AddRef,
        .Release = ddraw_Release,
        .Compact = ddraw_Compact,
        .CreateClipper = ddraw_CreateClipper,
        .CreatePalette = ddraw_CreatePalette,
        .CreateSurface = ddraw_CreateSurface,
        .DuplicateSurface = ddraw_DuplicateSurface,
        .EnumDisplayModes = ddraw_EnumDisplayModes,
        .EnumSurfaces = ddraw_EnumSurfaces,
        .FlipToGDISurface = ddraw_FlipToGDISurface,
        .GetCaps = ddraw_GetCaps,
        .GetDisplayMode = ddraw_GetDisplayMode,
        .GetFourCCCodes = ddraw_GetFourCCCodes,
        .GetGDISurface = ddraw_GetGDISurface,
        .GetMonitorFrequency = ddraw_GetMonitorFrequency,
        .GetScanLine = ddraw_GetScanLine,
        .GetVerticalBlankStatus = ddraw_GetVerticalBlankStatus,
        .Initialize = ddraw_Initialize,
        .RestoreDisplayMode = ddraw_RestoreDisplayMode,
        .SetCooperativeLevel = ddraw_SetCooperativeLevel,
        .SetDisplayMode = ddraw_SetDisplayMode,
        .WaitForVerticalBlank = ddraw_WaitForVerticalBlank,
        .GetAvailableVidMem = ddraw_GetAvailableVidMem,
        .GetSurfaceFromDC = ddraw_GetSurfaceFromDC,
        .RestoreAllSurfaces = ddraw_RestoreAllSurfaces,
        .TestCooperativeLevel = ddraw_TestCooperativeLevel,
        .GetDeviceIdentifier = ddraw_GetDeviceIdentifier,
    };

    self->renderer = renderer;

    return true;
}

void ddraw_iface__cleanup(ddraw_iface_t* self) {
    // Do nothing
}

#undef INTERFACE
#define INTERFACE IDirectDraw4
/*** IUnknown methods ***/
STDMETHODIMP ddraw_QueryInterface (THIS_ REFIID riid, LPVOID FAR * ppvObj) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP_(ULONG) ddraw_AddRef (THIS)  PURE {
    LOG_FUNC();
    
    auto self = (ddraw_iface_t*) This;
    return ++self->rc;
}

STDMETHODIMP_(ULONG) ddraw_Release (THIS) PURE {
    LOG_FUNC();

    auto self = (ddraw_iface_t*) This;
    return --self->rc;
}

/*** IDirectDraw methods ***/
STDMETHODIMP ddraw_Compact(THIS) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP ddraw_CreateClipper(THIS_ DWORD, LPDIRECTDRAWCLIPPER FAR*, IUnknown FAR * ) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP ddraw_CreatePalette(THIS_ DWORD, LPPALETTEENTRY, LPDIRECTDRAWPALETTE FAR*, IUnknown FAR * ) PURE {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP ddraw_CreateSurface(THIS_  LPDDSURFACEDESC2 desc, LPDIRECTDRAWSURFACE4 FAR * out, IUnknown FAR * reserved) PURE {
    LOG_FUNC();

    auto self = (ddraw_iface_t*) This;

    if (reserved != nullptr) {
        return DDERR_INVALIDPARAMS;
    }

    *out = SDL_malloc(sizeof(ddraw_surface_t));
    if (*out == nullptr) {
        LOG_ERROR("Failed to allocate ddraw_surface_t");
        return DDERR_OUTOFMEMORY;
    }

    if (!ddraw_surface__init((ddraw_surface_t*) *out, self, desc)) {
        LOG_ERROR("Failed to init ddraw_surface_t");
        return DDERR_INVALIDPARAMS;
    }

    return DD_OK;
}

STDMETHODIMP ddraw_DuplicateSurface( THIS_ LPDIRECTDRAWSURFACE4, LPDIRECTDRAWSURFACE4 FAR * ) PURE  {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP ddraw_EnumDisplayModes( THIS_ DWORD, LPDDSURFACEDESC2, LPVOID, LPDDENUMMODESCALLBACK2 ) PURE  {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP ddraw_EnumSurfaces(THIS_ DWORD, LPDDSURFACEDESC2, LPVOID,LPDDENUMSURFACESCALLBACK2 ) PURE  {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP ddraw_FlipToGDISurface(THIS) PURE  {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP ddraw_GetCaps( THIS_ LPDDCAPS, LPDDCAPS) PURE  {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP ddraw_GetDisplayMode( THIS_ LPDDSURFACEDESC2 out) PURE  {
    LOG_FUNC();

    auto self = (ddraw_iface_t*) This;

    out->dwFlags =
        DDSD_WIDTH |
        DDSD_HEIGHT
    ;

    int w, h;
    if (!SDL_GetWindowSize(self->renderer->window, &w, &h)) {
        return DDERR_INVALIDOBJECT;
    }

    out->dwHeight = w;
    out->dwWidth = h;
    out->lPitch = 0;
    out->dwBackBufferCount = 0;
    out->dwRefreshRate = 0;
    out->dwAlphaBitDepth = 0;
    out->dwReserved = 0;
    out->lpSurface = nullptr;
    out->ddckCKDestOverlay = (DDCOLORKEY) { 0 };
    out->ddckCKDestBlt = (DDCOLORKEY) { 0 };
    out->ddckCKSrcOverlay = (DDCOLORKEY) { 0 };
    out->ddckCKSrcBlt = (DDCOLORKEY) { 0 };
    out->ddpfPixelFormat = (DDPIXELFORMAT) { 0 };
    out->ddsCaps = (DDSCAPS2) { 0 };
    out->dwTextureStage = 0;

    return DD_OK;
}

STDMETHODIMP ddraw_GetFourCCCodes(THIS_  LPDWORD, LPDWORD ) PURE  {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP ddraw_GetGDISurface(THIS_ LPDIRECTDRAWSURFACE4 FAR *) PURE  {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP ddraw_GetMonitorFrequency(THIS_ LPDWORD) PURE  {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP ddraw_GetScanLine(THIS_ LPDWORD) PURE  {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP ddraw_GetVerticalBlankStatus(THIS_ LPBOOL ) PURE  {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP ddraw_Initialize(THIS_ GUID FAR *) PURE  {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP ddraw_RestoreDisplayMode(THIS) PURE  {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP ddraw_SetCooperativeLevel(THIS_ HWND, DWORD) PURE  {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP ddraw_SetDisplayMode(THIS_ DWORD, DWORD,DWORD, DWORD, DWORD) PURE  {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP ddraw_WaitForVerticalBlank(THIS_ DWORD, HANDLE ) PURE  {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

/*** Added in the v2 interface ***/
STDMETHODIMP ddraw_GetAvailableVidMem(THIS_ LPDDSCAPS2, LPDWORD, LPDWORD) PURE  {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

/*** Added in the V4 Interface ***/
STDMETHODIMP ddraw_GetSurfaceFromDC (THIS_ HDC, LPDIRECTDRAWSURFACE4 *) PURE  {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP ddraw_RestoreAllSurfaces(THIS) PURE  {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP ddraw_TestCooperativeLevel(THIS) PURE  {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

STDMETHODIMP ddraw_GetDeviceIdentifier(THIS_ LPDDDEVICEIDENTIFIER, DWORD ) PURE  {
    LOG_FUNC();
    return DDERR_UNSUPPORTED;
}

