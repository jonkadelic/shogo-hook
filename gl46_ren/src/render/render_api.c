#include "./render_api.h"

#include "logger.h"
#include "renderer.h"

RenderStruct_t* RENDER_STRUCT = nullptr;

uint32_t __cdecl r_Init(RenderStructInit_t* pInit) {
    LOG_FUNC();

    if (!renderer__init(&pInit->m_Mode, pInit->m_hWnd)) {
        return 1;
    }

    return 0;
}

void __cdecl r_Term(void) {
    LOG_FUNC();

    call_stats__log_all();
    call_stats__clear();

    renderer__reset();
}

void __cdecl r_SetSoftSky(SharedTexture_t** ppTex) {
    LOG_FUNC();
}

void __cdecl r_BindTexture(SharedTexture_t* pTex, uint8_t bTextureChanged) {
    LOG_FUNC();

    auto data = RENDER_STRUCT->GetTexture(pTex, 0);
    if (data != nullptr) {
        RENDER_STRUCT->FreeTexture(pTex);
    }
}

void __cdecl r_UnbindTexture(SharedTexture_t* pTex) {
    LOG_FUNC();
}

DBOOL __cdecl r_QueryDeletePalette(void* pPalette) {
    LOG_FUNC();
    return true;
}

DBOOL __cdecl r_SetMasterPalette(SharedTexture_t* pPalette) {
    LOG_FUNC();
    return true;
}

void* __cdecl r_CreateContext(void* pInit) {
    LOG_FUNC();
    return nullptr;
}

void __cdecl r_DeleteContext(void* pContext) {
    LOG_FUNC();
}

void __cdecl r_Clear(DRect_t* pRect, uint32_t flags) {
    LOG_FUNC();
    glClearColor(28.0 / 255.0, 39.0 / 255.0, 74.0 / 255.0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

DBOOL __cdecl r_Start3D(void) {
    LOG_FUNC();
    return renderer__start_3d();
}

DBOOL __cdecl r_End3D(void) {
    LOG_FUNC();
    return renderer__end_3d();
}

DBOOL __cdecl r_IsIn3D(void) {
    LOG_FUNC();
    return renderer__is_in_3d();
}

DBOOL __cdecl r_StartOptimized2D(void) {
    LOG_FUNC();
    return renderer__start_2d();
}

DBOOL __cdecl r_EndOptimized2D(void) {
    LOG_FUNC();
    return renderer__end_2d();
}

DBOOL __cdecl r_IsInOptimized2D(void) {
    LOG_FUNC();
    return renderer__is_in_2d();
}

uint32_t __cdecl r_RenderScene(SceneDesc_t* pSceneDesc) {
    LOG_FUNC();

    float aspect = (float) (pSceneDesc->m_Rect.right - pSceneDesc->m_Rect.left) / (float) (pSceneDesc->m_Rect.bottom - pSceneDesc->m_Rect.top);
    renderer__set_camera(pSceneDesc->m_Pos, pSceneDesc->m_Rotation, pSceneDesc->m_yFov, aspect);

    if (pSceneDesc->m_DrawMode == SceneDrawMode_Normal) {

    } else if (pSceneDesc->m_DrawMode == SceneDrawMode_ObjectList) {
        for (size_t i = 0; i < pSceneDesc->m_nObjectListSize; i++) {
            renderer__draw_object(pSceneDesc->m_pObjectList[i]);
        }
    }

    return true;
}

void __cdecl r_RenderCommand(uint32_t argc, char** argv) {
    LOG_FUNC();
}

void* __cdecl r_GetHook(char* pHook) {
    LOG_FUNC();
    
    if (strcmp(pHook, "LPDIRECTDRAW") == 0) {
        return &renderer__get_instance()->ddraw.iface;
    }
    if (strcmp(pHook, "BACKBUFFER") == 0) {
        return &renderer__get_instance()->ddraw.backbuffer;
    }

    return nullptr;
}

void __cdecl r_SwapBuffers(void) {
    LOG_FUNC();
    renderer__swap_buffers();
}

uint32_t __cdecl r_GetInfoFlags(void) {
    LOG_FUNC();
    return 0;
}

uint32_t __cdecl r_GetBufferFormat(void) {
    LOG_FUNC();
    return 0;
}

void* __cdecl r_CreateSurface(int32_t width, int32_t height) {
    LOG_FUNC();

    auto surfaces = renderer__get_surfaces();
    if (surfaces == nullptr) {
        return nullptr;
    }

    return surface_manager__create_surface(surfaces, width, height);
}

void __cdecl r_DeleteSurface(void* pSurface) {
    LOG_FUNC();

    if (pSurface == nullptr) {
        return;
    }

    auto surface = (surface_t*) pSurface;
    auto surfaces = renderer__get_surfaces();
    if (surfaces == nullptr) {
        return;
    }

    surface_manager__delete_surface(surfaces, surface->idx);
}

void __cdecl r_GetSurfaceInfo(void* pSurface, int32_t* pWidth, int32_t* pHeight, int32_t* pPitchBytes) {
    LOG_FUNC();

    if (pSurface == nullptr) {
        return;
    }

    auto surface = (surface_t*) pSurface;

    if (pWidth != nullptr) {
        *pWidth = surface->width;
    }
    if (pHeight != nullptr) {
        *pHeight = surface->height;
    }
    if (pPitchBytes != nullptr) {
        *pPitchBytes = surface->width * sizeof(uint16_t);
    }
}

void* __cdecl r_LockSurface(void* pSurface) {
    LOG_FUNC();

    if (pSurface == nullptr) {
        return nullptr;
    }

    auto surface = (surface_t*) pSurface;
    if (surface->locked) {
        return nullptr;
    }

    surface->locked = true;
    return surface->buffer.data;
}

void __cdecl r_UnlockSurface(void* pSurface) {
    LOG_FUNC();

    if (pSurface == nullptr) {
        return;
    }

    auto surface = (surface_t*) pSurface;
    surface->locked = false;
}

DBOOL __cdecl r_OptimizeSurface(void* pSurface, uint32_t transparentColor) {
    LOG_FUNC();
    return true;
}

void __cdecl r_UnoptimizeSurface(void* pSurface) {
    LOG_FUNC();
}

DBOOL __cdecl r_LockScreen(int32_t left, int32_t top, int32_t right, int32_t bottom, void** pData, int32_t* pPitch) {
    LOG_FUNC();

    auto r = renderer__get_instance();

    if (left < 0 || top < 0 || right < 0 || bottom < 0 || left >= r->screen.buffer.width || top >= r->screen.buffer.height || right > r->screen.buffer.width || bottom > r->screen.buffer.height) {
        return false;
    }
    if (left >= right || top >= bottom) {
        return false;
    }

    void* data = screen__lock(&r->screen);
    if (data == nullptr) {
        return false;
    }

    size_t width = right - left;
    *pPitch = r->screen.buffer.width * r->screen.buffer.bpp;

    size_t i = (top * (r->screen.buffer.width * r->screen.buffer.bpp)) + (left * r->screen.buffer.bpp);
    *pData = ((uint8_t*) data) + i;

    return true;
}

void __cdecl r_UnlockScreen(void) {
    LOG_FUNC();
}

void __cdecl r_BlitToScreen(BlitRequest_t* pRequest) {
    LOG_FUNC();

    blitter__blit_request(renderer__get_blitter(), pRequest);
}

void __cdecl r_MakeScreenShot(char const* pFilename) {
    LOG_FUNC();
}

void __cdecl r_ReadConsoleVariables(void) {
    LOG_FUNC();
}
