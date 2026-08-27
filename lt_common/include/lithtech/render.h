#pragma once

#include <stddef.h>

#include "./common.h"
#include "./texture.h"
#include "./shapes.h"
#include "./scene.h"
#include "./blit_request.h"

typedef struct RMode {
    bool m_bHardware;
    char m_RenderDLL[200];
    char m_InternalName[100];
    char m_Description[100];
    uint32_t m_Width, m_Height, m_BitDepth;
    struct RMode* m_pNext;
} RMode_t;
static_assert(sizeof(RMode_t) == 0x01A4);
static_assert(offsetof(RMode_t, m_bHardware) == 0x0000);
static_assert(offsetof(RMode_t, m_RenderDLL) == 0x0001);
static_assert(offsetof(RMode_t, m_InternalName) == 0x00C9);
static_assert(offsetof(RMode_t, m_Description) == 0x012D);
static_assert(offsetof(RMode_t, m_Width) == 0x0194);
static_assert(offsetof(RMode_t, m_Height) == 0x0198);
static_assert(offsetof(RMode_t, m_BitDepth) == 0x019C);
static_assert(offsetof(RMode_t, m_pNext) == 0x01A0);

typedef struct RenderStructInit {
    RMode_t m_Mode;
    void* m_hWnd;
} RenderStructInit_t;
static_assert(sizeof(RenderStructInit_t) == 0x01A8);
static_assert(offsetof(RenderStructInit_t, m_Mode) == 0x0000);
static_assert(offsetof(RenderStructInit_t, m_hWnd) == 0x01A4);

typedef struct RenderStruct {
    void* ProcessAttachment;
    void* GetTexture;
    void* FreeTexture;
    void* GetLightmap;
    void* GetPaletteColors;
    void* GetPaletteUserData;
    void* SetPaletteUserData;
    void* RunConsoleString;
    void* ConsolePrint;
    void* GetParameter;
    void* GetParameterValueFloat;
    void* GetParameterValueString;
    void* RendererPing;

    uint32_t m_Width;
    uint32_t m_Height;
    DBOOL m_bInitted;
    DBOOL m_bLoaded;
    uint32_t m_nIn3D;
    uint32_t m_nInOptimized2D;
    uint32_t m_SystemTextureMemory;

    uint32_t (*__cdecl Init)(RenderStructInit_t* pInit);
    void (*__cdecl Term)(void);
    void (*__cdecl SetSoftSky)(SharedTexture_t** ppTex);
    void (*__cdecl BindTexture)(SharedTexture_t* pTex, uint8_t bTextureChanged);
    void (*__cdecl UnbindTexture)(SharedTexture_t* pTex);
    DBOOL (*__cdecl QueryDeletePalette)(void* pPalette);
    DBOOL (*__cdecl SetMasterPalette)(SharedTexture_t* pPalette);
    void* (*__cdecl CreateContext)(void* pInit);
    void (*__cdecl DeleteContext)(void* pContext);
    void (*__cdecl Clear)(DRect_t* pRect, uint32_t flags);
    DBOOL (*__cdecl Start3D)(void);
    DBOOL (*__cdecl End3D)(void);
    DBOOL (*__cdecl IsIn3D)(void);
    DBOOL (*__cdecl StartOptimized2D)(void);
    DBOOL (*__cdecl EndOptimized2D)(void);
    DBOOL (*__cdecl IsInOptimized2D)(void);
    uint32_t (*__cdecl RenderScene)(SceneDesc_t* pSceneDesc);
    void (*__cdecl RenderCommand)(uint32_t argc, char** argv);
    void* (*__cdecl GetHook)(char* pHook);
    void (*__cdecl SwapBuffers)(void);
    uint32_t (*__cdecl GetInfoFlags)(void);
    uint32_t (*__cdecl GetBufferFormat)(void);
    void* (*__cdecl CreateSurface)(int32_t width, int32_t height);
    void (*__cdecl DeleteSurface)(void* pSurface);
    void (*__cdecl GetSurfaceInfo)(void* pSurface, int32_t* pWidth, int32_t* pHeight, int32_t* pPitchBytes);
    void* (*__cdecl LockSurface)(void* pSurface);
    void (*__cdecl UnlockSurface)(void* pSurface);
    DBOOL (*__cdecl OptimizeSurface)(void* pSurface, uint32_t transparentColor);
    void (*__cdecl UnoptimizeSurface)(void* pSurface);
    DBOOL (*__cdecl LockScreen)(int32_t left, int32_t top, int32_t right, int32_t bottom, void** pData, int32_t* pPitch);
    void (*__cdecl UnlockScreen)(void);
    void (*__cdecl BlitToScreen)(BlitRequest_t* pRequest);
    void (*__cdecl MakeScreenShot)(char const* pFilename);
    void (*__cdecl ReadConsoleVariables)(void);
    uint32_t _[4];
    uint32_t m_DontClearMarker;
    float m_GlobalLightDir[3];
    float m_GlobalLightColor[3];
    float m_GlobalLightConvertToAmbient;    
} RenderStruct_t;
