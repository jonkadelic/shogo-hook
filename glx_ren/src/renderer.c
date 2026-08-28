#include "./renderer.h"

#include <windows.h>
#include <stdio.h>

#include <SDL3/SDL.h>
#include <glad/glad.h>

#include "debug.h"
#include "blit.h"
#include "object.h"

static void init_display(RMode_t const* rmode);

renderer_t* renderer__get(void) {
    static renderer_t* RENDERER = nullptr;
    if (RENDERER == nullptr) {
        RENDERER = calloc(1, sizeof(renderer_t));
        DEBUG_ASSERT(RENDERER != nullptr);

        if (!surface_manager__init(&RENDERER->surfaces)) {
            DEBUG_ASSERT(false);
        }
    }

    return RENDERER;
}

uint32_t __cdecl r_Init(RenderStructInit_t* pInit) {
    DEBUG_PRINT_FUN();

    auto renderer = renderer__get();
    if (renderer->init_run) {
        return 0;
    }

    renderer->init_run = true;

// #ifdef DEBUG
    if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
        AllocConsole();
    }

    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
    setvbuf(stdout, nullptr, _IONBF, 0);
// #endif

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

    SDL_PropertiesID props = SDL_CreateProperties();
    if (props == 0) {
        return 1;
    }
    DEBUG_PRINT("Created properties.");

    DEBUG_ASSERT(pInit->m_hWnd != nullptr);

    if (!SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_WIN32_HWND_POINTER, pInit->m_hWnd)) {
        SDL_DestroyProperties(props);
        DEBUG_HANG();
        return 1;
    }
    if (!SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN, true)) {
        SDL_DestroyProperties(props);
        DEBUG_HANG();
        return 1;
    }
    DEBUG_PRINT("Set properties.");

    renderer->window = SDL_CreateWindowWithProperties(props);
    // renderer->window = SDL_CreateWindow("LithTech", 640, 480, SDL_WINDOW_OPENGL);
    SDL_DestroyProperties(props);
    if (renderer->window == nullptr) {
        printf("Failed to create SDL window: %s\n", SDL_GetError());
        DEBUG_HANG();
        return 1;
    }
    DEBUG_PRINT("Created window.");

    init_display(&pInit->m_Mode);
    DEBUG_PRINT("Initialized display.");

    renderer->gl_context = SDL_GL_CreateContext(renderer->window);
    if (renderer->gl_context == nullptr) {
        printf("Failed to create OpenGL context: %s\n", SDL_GetError());
        SDL_DestroyWindow(renderer->window);
        DEBUG_HANG();
        return 1;
    }
    DEBUG_PRINT("Created OpenGL context.");

    int gl_version = gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress);
    if (gl_version == 0) {
        printf("Failed to load GLAD.\n");
        SDL_GL_DestroyContext(renderer->gl_context);
        SDL_DestroyWindow(renderer->window);
        DEBUG_HANG();
        return 1;
    }
    DEBUG_PRINT("Successfully loaded OpenGL via GLAD.");

    glViewport(0, 0, pInit->m_Mode.m_Width, pInit->m_Mode.m_Height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, pInit->m_Mode.m_Width, pInit->m_Mode.m_Height, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    return 0;
}

void __cdecl r_Term(void) {
    DEBUG_PRINT_FUN();
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
    glClearColor(28.0 / 255.0, 39.0 / 255.0, 74.0 / 255.0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

DBOOL __cdecl r_Start3D(void) {
    DEBUG_PRINT_FUN();

    auto r = renderer__get();

    if (r->in_3d) {
        return true;
    }

    r->in_3d = true;
    return true;
}

DBOOL __cdecl r_End3D(void) {
    DEBUG_PRINT_FUN();
    
    auto r = renderer__get();

    if (!r->in_3d) {
        return false;
    }

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

    if (pSceneDesc->m_DrawMode == SceneDrawMode_Normal) {
        printf("Drawing normal scene!\n");
    } else if (pSceneDesc->m_DrawMode == SceneDrawMode_ObjectList) {
        printf("Drawing object list scene! (%u objects)\n", pSceneDesc->m_nObjectListSize);

        for (size_t i = 0; i < pSceneDesc->m_nObjectListSize; i++) {
            object__draw(pSceneDesc->m_pObjectList[i]);
        }
    } else {
        printf("Unexpected scene draw mode! (%d)\n", pSceneDesc->m_DrawMode);
    }

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

    auto renderer = renderer__get();

    SDL_GL_SwapWindow(renderer->window);
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
    DEBUG_PRINT_FUN();

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
    DEBUG_PRINT_FUN();

    auto surface = (surface_t*) pSurface;
    
    *pWidth = surface->width;
    *pHeight = surface->height;
    *pPitchBytes = surface->width * sizeof(uint16_t);
}

void* __cdecl r_LockSurface(void* pSurface) {
    // DEBUG_PRINT_FUN();

    auto surface = (surface_t*) pSurface;

    surface->locked = true;

    return surface->data;
}

void __cdecl r_UnlockSurface(void* pSurface) {
    // DEBUG_PRINT_FUN();

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

    blit__blit_to_screen(pRequest);
}

void __cdecl r_MakeScreenShot(char const* pFilename) {
    DEBUG_PRINT_FUN();
}

void __cdecl r_ReadConsoleVariables(void) {
    DEBUG_PRINT_FUN();
}

static void init_display(RMode_t const* rmode) {
    renderer_t* const renderer = renderer__get();

    if (strncmp(rmode->m_InternalName, "windowed", sizeof(rmode->m_InternalName)) == 0) {
        if (!SDL_SetWindowSize(renderer->window, rmode->m_Width, rmode->m_Height)) {
            goto err;
        }
        if (!SDL_SetWindowFullscreen(renderer->window, false)) {
            goto err;
        }
    } else {
        int num_display_modes = 0;
        SDL_DisplayMode** display_modes = nullptr;

        SDL_DisplayID display_id;
        if (sscanf(rmode->m_InternalName, "sdl:%u", &display_id) != 1) {
            goto err;
        }

        bool set = false;
        display_modes = SDL_GetFullscreenDisplayModes(display_id, &num_display_modes);
        for (int i = 0; i < num_display_modes; i++) {
            SDL_DisplayMode* mode = display_modes[i];

            if (rmode->m_Width == mode->w && rmode->m_Height == mode->h && rmode->m_BitDepth == SDL_BITSPERPIXEL(mode->format)) {
                set = true;
                if (!SDL_SetWindowFullscreenMode(renderer->window, mode)) {
                    goto err;
                }
                if (!SDL_SetWindowFullscreen(renderer->window, true)) {
                    goto err;
                }
            }
        }

        if (!set) {
            goto err;
        }
    }

    return;

err:
    SDL_SetWindowSize(renderer->window, 640, 480);
    SDL_SetWindowFullscreen(renderer->window, false);
}