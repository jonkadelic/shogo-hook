#include "./render_api.h"

#include <windows.h>

#include "util/debug.h"
#include "render/renderer.h"

uint32_t __cdecl r_Init(RenderStructInit_t* pInit) {
    DEBUG_PRINT_FUN();

    auto r = renderer__get();
    renderer__cleanup(r);
    if (!renderer__init(r, pInit->m_hWnd, &pInit->m_Mode)) {
        return 1;
    }

    return 0;
}

void __cdecl r_Term(void) {
    DEBUG_PRINT_FUN();

    // renderer__cleanup(renderer__get());
}

void __cdecl r_SetSoftSky(SharedTexture_t** ppTex) {
    DEBUG_PRINT_FUN();
}

void __cdecl r_BindTexture(SharedTexture_t* pTex, uint8_t bTextureChanged) {
    DEBUG_PRINT_FUN();
}

void __cdecl r_UnbindTexture(SharedTexture_t* pTex) {
    DEBUG_PRINT_FUN();
}

DBOOL __cdecl r_QueryDeletePalette(void* pPalette) {
    DEBUG_PRINT_FUN();
    return true;
}

DBOOL __cdecl r_SetMasterPalette(SharedTexture_t* pPalette) {
    DEBUG_PRINT_FUN();
    return true;
}

void* __cdecl r_CreateContext(void* pInit) {
    DEBUG_PRINT_FUN();

    return nullptr;
}

void __cdecl r_DeleteContext(void* pContext) {
    DEBUG_PRINT_FUN();
}

void __cdecl r_Clear(DRect_t* pRect, uint32_t flags) {
    DEBUG_PRINT_FUN();

    auto r = renderer__get();
    renderer__clear(r, 28.0f / 255.0f, 39.0f / 255.0f, 74.0f / 255.0f, 1.0f);
}

DBOOL __cdecl r_Start3D(void) {
    DEBUG_PRINT_FUN();

    auto r = renderer__get();

    if (r->in_3d) {
        return true;
    }

    renderer__start_draw(r);

    r->in_3d = true;
    return true;
}

DBOOL __cdecl r_End3D(void) {
    DEBUG_PRINT_FUN();
    
    auto r = renderer__get();

    if (!r->in_3d) {
        return false;
    }

    renderer__end_draw(r);

    r->in_3d = false;
    return true;
}

DBOOL __cdecl r_IsIn3D(void) {
    DEBUG_PRINT_FUN();
    
    return renderer__get()->in_3d;
}

DBOOL __cdecl r_StartOptimized2D(void) {
    DEBUG_PRINT_FUN();

    auto r = renderer__get();

    if (r->in_2d) {
        return true;
    }

    r->in_2d = true;
    return true;
}

DBOOL __cdecl r_EndOptimized2D(void) {
    DEBUG_PRINT_FUN();
    
    auto r = renderer__get();

    if (!r->in_2d) {
        return false;
    }

    r->in_2d = false;
    return true;
}

DBOOL __cdecl r_IsInOptimized2D(void) {
    DEBUG_PRINT_FUN();

    return renderer__get()->in_2d;
}

uint32_t __cdecl r_RenderScene(SceneDesc_t* pSceneDesc) {
    DEBUG_PRINT_FUN();

    auto r = renderer__get();
    renderer__draw_scene(r, pSceneDesc);

    return 0;
}

void __cdecl r_RenderCommand(uint32_t argc, char** argv) {
    DEBUG_PRINT_FUN();
}

void* __cdecl r_GetHook(char* pHook) {
    DEBUG_PRINT_FUN();

    return nullptr;
}

void __cdecl r_SwapBuffers(void) {
    DEBUG_PRINT_FUN();
}

uint32_t __cdecl r_GetInfoFlags(void) {
    DEBUG_PRINT_FUN();
    return 0;
}

uint32_t __cdecl r_GetBufferFormat(void) {
    // DEBUG_PRINT_FUN();
    return 0;
}

void* __cdecl r_CreateSurface(int32_t width, int32_t height) {
    // DEBUG_PRINT_FUN();

    auto renderer = renderer__get();
    auto surface = surface_manager__create_surface(&renderer->surfaces, width, height);

    return surface;
}

void __cdecl r_DeleteSurface(void* pSurface) {
    // DEBUG_PRINT_FUN();

    auto renderer = renderer__get();
    auto surface = (surface_t*) pSurface;

    surface_manager__delete_surface(&renderer->surfaces, surface->idx);
}

void __cdecl r_GetSurfaceInfo(void* pSurface, int32_t* pWidth, int32_t* pHeight, int32_t* pPitchBytes) {
    // DEBUG_PRINT_FUN();

    auto surface = (surface_t*) pSurface;
    
    *pWidth = surface->width;
    *pHeight = surface->height;
    *pPitchBytes = surface->width * sizeof(uint16_t);
}

void* __cdecl r_LockSurface(void* pSurface) {
    auto surface = (surface_t*) pSurface;

    surface->locked = true;

    return surface->extern_data;
}

void __cdecl r_UnlockSurface(void* pSurface) {
    // Do nothing
}

DBOOL __cdecl r_OptimizeSurface(void* pSurface, uint32_t transparentColor) {
    DEBUG_PRINT_FUN();
    return true;
}

void __cdecl r_UnoptimizeSurface(void* pSurface) {
    DEBUG_PRINT_FUN();
}

DBOOL __cdecl r_LockScreen(int32_t left, int32_t top, int32_t right, int32_t bottom, void** pData, int32_t* pPitch) {
    DEBUG_PRINT_FUN();
    return false;
}

void __cdecl r_UnlockScreen(void) {
    DEBUG_PRINT_FUN();
}

void __cdecl r_BlitToScreen(BlitRequest_t* pRequest) {
    DEBUG_PRINT_FUN();

    auto r = renderer__get();
    renderer__blit_to_screen(r, pRequest);
}

void __cdecl r_MakeScreenShot(char const* pFilename) {
    DEBUG_PRINT_FUN();
}

void __cdecl r_ReadConsoleVariables(void) {
    DEBUG_PRINT_FUN();
}
