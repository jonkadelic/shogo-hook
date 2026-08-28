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
static_assert(sizeof(RMode_t) == 0x01A4); // defined in glx.ren::GetSupportedModes()
static_assert(offsetof(RMode_t, m_bHardware) == 0x0000);
static_assert(offsetof(RMode_t, m_RenderDLL) == 0x0001);
static_assert(offsetof(RMode_t, m_InternalName) == 0x00c9);
static_assert(offsetof(RMode_t, m_Description) == 0x012d);
static_assert(offsetof(RMode_t, m_Width) == 0x0194);
static_assert(offsetof(RMode_t, m_Height) == 0x0198);
static_assert(offsetof(RMode_t, m_BitDepth) == 0x019c);
static_assert(offsetof(RMode_t, m_pNext) == 0x01a0);

typedef struct RenderStructInit {
    RMode_t m_Mode;
    void* m_hWnd;
} RenderStructInit_t;
static_assert(sizeof(RenderStructInit_t) == 0x01a8);
static_assert(offsetof(RenderStructInit_t, m_Mode) == 0x0000);
static_assert(offsetof(RenderStructInit_t, m_hWnd) == 0x01a4);

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
    uint32_t unk_d8[4];
    uint32_t m_DontClearMarker;
    DVector_t m_GlobalLightDir;
    DVector_t m_GlobalLightColor;
    float m_GlobalLightConvertToAmbient;    
} RenderStruct_t;
static_assert(sizeof(RenderStruct_t) == 0x0108);
static_assert(offsetof(RenderStruct_t, ProcessAttachment) == 0x0000);
static_assert(offsetof(RenderStruct_t, GetTexture) == 0x0004);
static_assert(offsetof(RenderStruct_t, FreeTexture) == 0x0008);
static_assert(offsetof(RenderStruct_t, GetLightmap) == 0x000c);
static_assert(offsetof(RenderStruct_t, GetPaletteColors) == 0x0010);
static_assert(offsetof(RenderStruct_t, GetPaletteUserData) == 0x0014);
static_assert(offsetof(RenderStruct_t, SetPaletteUserData) == 0x0018);
static_assert(offsetof(RenderStruct_t, RunConsoleString) == 0x001c);
static_assert(offsetof(RenderStruct_t, ConsolePrint) == 0x0020);
static_assert(offsetof(RenderStruct_t, GetParameter) == 0x0024);
static_assert(offsetof(RenderStruct_t, GetParameterValueFloat) == 0x0028);
static_assert(offsetof(RenderStruct_t, GetParameterValueString) == 0x002c);
static_assert(offsetof(RenderStruct_t, RendererPing) == 0x0030);
static_assert(offsetof(RenderStruct_t, m_Width) == 0x0034);
static_assert(offsetof(RenderStruct_t, m_Height) == 0x0038);
static_assert(offsetof(RenderStruct_t, m_bInitted) == 0x003c);
static_assert(offsetof(RenderStruct_t, m_bLoaded) == 0x0040);
static_assert(offsetof(RenderStruct_t, m_nIn3D) == 0x0044);
static_assert(offsetof(RenderStruct_t, m_nInOptimized2D) == 0x0048);
static_assert(offsetof(RenderStruct_t, m_SystemTextureMemory) == 0x004c);
static_assert(offsetof(RenderStruct_t, Init) == 0x0050);
static_assert(offsetof(RenderStruct_t, Term) == 0x0054);
static_assert(offsetof(RenderStruct_t, SetSoftSky) == 0x0058);
static_assert(offsetof(RenderStruct_t, BindTexture) == 0x005c);
static_assert(offsetof(RenderStruct_t, UnbindTexture) == 0x0060);
static_assert(offsetof(RenderStruct_t, QueryDeletePalette) == 0x0064);
static_assert(offsetof(RenderStruct_t, SetMasterPalette) == 0x0068);
static_assert(offsetof(RenderStruct_t, CreateContext) == 0x006c);
static_assert(offsetof(RenderStruct_t, DeleteContext) == 0x0070);
static_assert(offsetof(RenderStruct_t, Clear) == 0x0074);
static_assert(offsetof(RenderStruct_t, Start3D) == 0x0078);
static_assert(offsetof(RenderStruct_t, End3D) == 0x007c);
static_assert(offsetof(RenderStruct_t, IsIn3D) == 0x0080);
static_assert(offsetof(RenderStruct_t, StartOptimized2D) == 0x0084);
static_assert(offsetof(RenderStruct_t, EndOptimized2D) == 0x0088);
static_assert(offsetof(RenderStruct_t, IsInOptimized2D) == 0x008c);
static_assert(offsetof(RenderStruct_t, RenderScene) == 0x0090);
static_assert(offsetof(RenderStruct_t, RenderCommand) == 0x0094);
static_assert(offsetof(RenderStruct_t, GetHook) == 0x0098);
static_assert(offsetof(RenderStruct_t, SwapBuffers) == 0x009c);
static_assert(offsetof(RenderStruct_t, GetInfoFlags) == 0x00a0);
static_assert(offsetof(RenderStruct_t, GetBufferFormat) == 0x00a4);
static_assert(offsetof(RenderStruct_t, CreateSurface) == 0x00a8);
static_assert(offsetof(RenderStruct_t, DeleteSurface) == 0x00ac);
static_assert(offsetof(RenderStruct_t, GetSurfaceInfo) == 0x00b0);
static_assert(offsetof(RenderStruct_t, LockSurface) == 0x00b4);
static_assert(offsetof(RenderStruct_t, UnlockSurface) == 0x00b8);
static_assert(offsetof(RenderStruct_t, OptimizeSurface) == 0x00bc);
static_assert(offsetof(RenderStruct_t, UnoptimizeSurface) == 0x00c0);
static_assert(offsetof(RenderStruct_t, LockScreen) == 0x00c4);
static_assert(offsetof(RenderStruct_t, UnlockScreen) == 0x00c8);
static_assert(offsetof(RenderStruct_t, BlitToScreen) == 0x00cc);
static_assert(offsetof(RenderStruct_t, MakeScreenShot) == 0x00d0);
static_assert(offsetof(RenderStruct_t, ReadConsoleVariables) == 0x00d4);
static_assert(offsetof(RenderStruct_t, m_DontClearMarker) == 0x00e8);
static_assert(offsetof(RenderStruct_t, m_GlobalLightDir) == 0x00ec);
static_assert(offsetof(RenderStruct_t, m_GlobalLightColor) == 0x00f8);
static_assert(offsetof(RenderStruct_t, m_GlobalLightConvertToAmbient) == 0x0104);