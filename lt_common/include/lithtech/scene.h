#pragma once

#include "lithtech/common.h"
#include "lithtech/shapes.h"
#include "lithtech/object/object.h"
#include "lithtech/render.h"

typedef struct SkyDef {
    DVector_t m_Min;
    DVector_t m_Max;
    DVector_t m_ViewMin;
    DVector_t m_ViewMax;
} SkyDef_t;
static_assert(sizeof(SkyDef_t) == 0x30);
static_assert(offsetof(SkyDef_t, m_Min) == 0x00);
static_assert(offsetof(SkyDef_t, m_Max) == 0x0c);
static_assert(offsetof(SkyDef_t, m_ViewMin) == 0x18);
static_assert(offsetof(SkyDef_t, m_ViewMax) == 0x24);

typedef struct ModelHookData {
    DObject_t const* m_hObject;
    uint32_t m_Flags;
    uint32_t m_ObjectFlags;
    DVector_t* m_LightAdd;
} ModelHookData_t;

typedef enum SceneDrawMode : uint32_t {
    SceneDrawMode_Normal = 1,
    SceneDrawMode_ObjectList = 2,
} SceneDrawMode_t;

typedef struct SceneDesc {
    SceneDrawMode_t m_DrawMode;
    void* unk_04;
    void* unk_08;
    void* unk_0c;
    void* unk_10;
    void* unk_14;
    void* unk_18;
    DVector_t m_GlobalModelLightAdd;
    DVector_t m_GlobalModelLightScale;
    RenderContext_t* m_hRenderContext;
    DVector_t m_GlobalLightColor;
    DVector_t m_GlobalLightAdd;
    DVector_t m_GlobalLightScale;
    DVector_t unk_5c;
    float m_FrameTime;
    uint32_t m_CurObjectFrameCode;
    SkyDef_t m_SkyDef;
    DObject_t** m_pSkyObjects;
    uint32_t m_nSkyObjects;
    DRect_t m_Rect;
    float m_xFov;
    float m_yFov;
    float unk_c0;
    DVector_t m_Pos;
    DRotation_t m_Rotation;
    DObject_t** m_pObjectList;
    uint32_t m_nObjectListSize;
    void (*m_ModelHookFn)(ModelHookData_t* data, void* user);
    void* m_ModelHookUser;
} SceneDesc_t;
static_assert(sizeof(SceneDesc_t) == 0xf0);
static_assert(offsetof(SceneDesc_t, m_DrawMode) == 0x00);
static_assert(offsetof(SceneDesc_t, m_GlobalModelLightAdd) == 0x1c);
static_assert(offsetof(SceneDesc_t, m_GlobalModelLightScale) == 0x28);
static_assert(offsetof(SceneDesc_t, m_hRenderContext) == 0x34);
static_assert(offsetof(SceneDesc_t, m_GlobalLightColor) == 0x38);
static_assert(offsetof(SceneDesc_t, m_GlobalLightAdd) == 0x44);
static_assert(offsetof(SceneDesc_t, m_GlobalLightScale) == 0x50);
static_assert(offsetof(SceneDesc_t, m_CurObjectFrameCode) == 0x6c);
static_assert(offsetof(SceneDesc_t, m_SkyDef) == 0x70);
static_assert(offsetof(SceneDesc_t, m_pSkyObjects) == 0xa0);
static_assert(offsetof(SceneDesc_t, m_nSkyObjects) == 0xa4);
static_assert(offsetof(SceneDesc_t, m_Rect) == 0xa8);
static_assert(offsetof(SceneDesc_t, m_xFov) == 0xb8);
static_assert(offsetof(SceneDesc_t, m_yFov) == 0xbc);
static_assert(offsetof(SceneDesc_t, m_Pos) == 0xc4);
static_assert(offsetof(SceneDesc_t, m_Rotation) == 0xd0);
static_assert(offsetof(SceneDesc_t, m_pObjectList) == 0xe0);
static_assert(offsetof(SceneDesc_t, m_nObjectListSize) == 0xe4);
static_assert(offsetof(SceneDesc_t, m_ModelHookFn) == 0xe8);
static_assert(offsetof(SceneDesc_t, m_ModelHookUser) == 0xec);
