#pragma once

#include <stdint.h>

#include <SDL3/SDL.h>

#include <lithtech/lithtech.h>

#include "surfaces.h"

typedef struct renderer {
    bool init_run;
    SDL_Window* window;
    SDL_GLContext gl_context;
    bool in_3d;
    bool in_2d;
    surface_manager_t surfaces;
} renderer_t;

renderer_t* renderer__get(void);

uint32_t __cdecl r_Init(RenderStructInit_t* pInit);
void __cdecl r_Term(void);
void __cdecl r_SetSoftSky(SharedTexture_t** ppTex);
void __cdecl r_BindTexture(SharedTexture_t* pTex, uint8_t bTextureChanged);
void __cdecl r_UnbindTexture(SharedTexture_t* pTex);
DBOOL __cdecl r_QueryDeletePalette(void* pPalette);
DBOOL __cdecl r_SetMasterPalette(SharedTexture_t* pPalette);
void* __cdecl r_CreateContext(void* pInit);
void __cdecl r_DeleteContext(void* pContext);
void __cdecl r_Clear(DRect_t* pRect, uint32_t flags);
DBOOL __cdecl r_Start3D(void);
DBOOL __cdecl r_End3D(void);
DBOOL __cdecl r_IsIn3D(void);
DBOOL __cdecl r_StartOptimized2D(void);
DBOOL __cdecl r_EndOptimized2D(void);
DBOOL __cdecl r_IsInOptimized2D(void);
uint32_t __cdecl r_RenderScene(SceneDesc_t* pSceneDesc);
void __cdecl r_RenderCommand(uint32_t argc, char** argv);
void* __cdecl r_GetHook(char* pHook);
void __cdecl r_SwapBuffers(void);
uint32_t __cdecl r_GetInfoFlags(void);
uint32_t __cdecl r_GetBufferFormat(void);
void* __cdecl r_CreateSurface(int32_t width, int32_t height);
void __cdecl r_DeleteSurface(void* pSurface);
void __cdecl r_GetSurfaceInfo(void* pSurface, int32_t* pWidth, int32_t* pHeight, int32_t* pPitchBytes);
void* __cdecl r_LockSurface(void* pSurface);
void __cdecl r_UnlockSurface(void* pSurface);
DBOOL __cdecl r_OptimizeSurface(void* pSurface, uint32_t transparentColor);
void __cdecl r_UnoptimizeSurface(void* pSurface);
DBOOL __cdecl r_LockScreen(int32_t left, int32_t top, int32_t right, int32_t bottom, void** pData, int32_t* pPitch);
void __cdecl r_UnlockScreen(void);
void __cdecl r_BlitToScreen(BlitRequest_t* pRequest);
void __cdecl r_MakeScreenShot(char const* pFilename);
void __cdecl r_ReadConsoleVariables(void);