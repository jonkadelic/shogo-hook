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

    auto r = renderer__get_instance();
    SDL_SetWindowMouseGrab(r->window, false);
    SDL_ShowCursor();
}

void __cdecl r_SetSoftSky(SharedTexture_t** ppTex) {
    LOG_FUNC();
}

void __cdecl r_BindTexture(SharedTexture_t* pTex, uint8_t bTextureChanged) {
    LOG_FUNC();

    LOG_INFO("Binding texture at \"%s\"", pTex->m_pFile->m_Filename);
    shared_texture_manager__get_texture(renderer__get_shared_textures(), pTex);
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

RenderContext_t* __cdecl r_CreateContext(RenderContextInit_t* pInit) {
    LOG_FUNC();

    auto r = renderer__get_instance();

    r->render_context = SDL_calloc(1, sizeof(RenderContext_t));
    if (r->render_context == nullptr) {
        return nullptr;
    }

    r->render_context->m_pMainWorld = pInit->m_pMainWorld;
    r->render_context->m_CurFrameCode = 0xffff;
    
    return r->render_context;
}

void __cdecl r_DeleteContext(RenderContext_t* pContext) {
    LOG_FUNC();

    auto r = renderer__get_instance();

    SDL_free(r->render_context);
    r->render_context = nullptr;
}

void __cdecl r_Clear(DRect_t* pRect, uint32_t flags) {
    LOG_FUNC();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
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

    auto r = renderer__get_instance();
    if (!r->in_3d) {
        return 0;
    }
    if (r->in_2d) {
        LOG_WARNING("Tried to render 3D while in 2D mode");
        return 0;
    }

    float aspect = (float) (pSceneDesc->m_Rect.right - pSceneDesc->m_Rect.left) / (float) (pSceneDesc->m_Rect.bottom - pSceneDesc->m_Rect.top);
    renderer__set_camera(pSceneDesc->m_Pos, pSceneDesc->m_Rotation, pSceneDesc->m_yFov + r->yfov_mod, aspect);

    if (pSceneDesc->m_hRenderContext != nullptr && pSceneDesc->m_hRenderContext->m_pMainWorld != nullptr) {
        if (r->world == nullptr) {
            r->world = SDL_malloc(sizeof(world_renderer_t));
            if (r->world == nullptr) {
                LOG_ERROR("Failed to alloc world");
                return 0;
            }

            if (!world_renderer__init(r->world, pSceneDesc->m_hRenderContext->m_pMainWorld)) {
                LOG_ERROR("Failed to init world");
                return 0;
            }
        }
    }

    if (pSceneDesc->m_DrawMode != SceneDrawMode_ObjectList) {
        world_renderer__draw(r->world, pSceneDesc->m_hRenderContext->m_pMainWorld);
    }

    for (size_t i = 0; i < pSceneDesc->m_nObjectListSize; i++) {
        renderer__draw_object(pSceneDesc->m_pObjectList[i]);
    }

    // if (pSceneDesc->m_hRenderContext != nullptr && pSceneDesc->m_hRenderContext != nullptr && pSceneDesc->m_hRenderContext->m_pMainWorld != nullptr) {
    //     for (size_t i = 0; i < pSceneDesc->m_hRenderContext->m_pMainWorld->m_pWorldBsp->m_nWorldModels; i++) {
    //         auto object_list = &pSceneDesc->m_hRenderContext->m_pMainWorld->m_pWorldBsp->m_WorldModels[i];

    //         auto current_list = object_list->m_pNext;
    //         while (current_list != object_list) {
    //             SDL_assert(current_list->m_pObject->m_ObjectType < NumObjectTypes);
    //             renderer__draw_object(current_list->m_pObject);
    //             current_list = current_list->m_pNext;
    //         }
    //     }
    // }

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

    auto r = renderer__get_instance();
    screen__draw(&r->screen);
    screen__clear(&r->screen);

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
    }

    renderer__swap_buffers();
    SDL_SetWindowMouseGrab(r->window, true);
    SDL_HideCursor();
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

    auto rsurfaces = renderer__get_rsurfaces();
    if (rsurfaces == nullptr) {
        return nullptr;
    }

    return (void*) rsurface_manager__create_rsurface(rsurfaces, width, height);
}

void __cdecl r_DeleteSurface(void* pSurface) {
    LOG_FUNC();

    rsurface_id_t id = (rsurface_id_t) pSurface;
    if (id == RSURFACE_ID_INVALID) return;

    auto rsurfaces = renderer__get_rsurfaces();
    if (rsurfaces == nullptr) {
        return;
    }

    rsurface_manager__delete_rsurface(rsurfaces, id);
}

void __cdecl r_GetSurfaceInfo(void* pSurface, int32_t* pWidth, int32_t* pHeight, int32_t* pPitchBytes) {
    LOG_FUNC();

    rsurface_id_t id = (rsurface_id_t) pSurface;
    if (id == RSURFACE_ID_INVALID) return;

    auto rsurfaces = renderer__get_rsurfaces();
    if (rsurfaces == nullptr) {
        return;
    }

    auto rsurface = rsurface_manager__get_rsurface(rsurfaces, id);

    if (pWidth != nullptr) {
        *pWidth = rsurface->width;
    }
    if (pHeight != nullptr) {
        *pHeight = rsurface->height;
    }
    if (pPitchBytes != nullptr) {
        *pPitchBytes = rsurface->width * sizeof(uint16_t);
    }
}

void* __cdecl r_LockSurface(void* pSurface) {
    LOG_FUNC();

    rsurface_id_t id = (rsurface_id_t) pSurface;
    if (id == RSURFACE_ID_INVALID) return nullptr;

    auto rsurfaces = renderer__get_rsurfaces();
    if (rsurfaces == nullptr) {
        return nullptr;
    }

    auto rsurface = rsurface_manager__get_rsurface(rsurfaces, id);
    if (rsurface == nullptr) {
        return nullptr;
    }

    rsurface->locked = true;
    return rsurface->buffer.pixel_data;
}

void __cdecl r_UnlockSurface(void* pSurface) {
    LOG_FUNC();
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

    pixel_buffer_t* buf = &r->screen.buffer;

    if (left < 0 || top < 0 || right < 0 || bottom < 0 || left >= buf->width || top >= buf->height || right > buf->width || bottom > buf->height) {
        return false;
    }
    if (left >= right || top >= bottom) {
        return false;
    }

    void* data = screen__lock(&r->screen);

    auto screen_bpp = color_format__get_bpp(buf->format);

    size_t width = right - left;
    *pPitch = buf->width * screen_bpp;

    size_t i = (top * (buf->width * screen_bpp)) + (left * screen_bpp);
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
