#pragma once

#include "./common.h"

typedef struct TextureData {
    uint32_t unk_00[18];
    uint32_t m_Width;
    uint32_t m_Height;
    uint32_t unk_50[2];
    uint8_t* m_pData;
} TextureData_t;
static_assert(sizeof(TextureData_t) == 0x5c);
static_assert(offsetof(TextureData_t, m_Width) == 0x48);
static_assert(offsetof(TextureData_t, m_Height) == 0x4c);
static_assert(offsetof(TextureData_t, m_pData) == 0x58);

typedef struct TBuild {
    uint32_t unk_00;
    TextureData_t* m_pTextureData;
    uint32_t unk_08;
} TBuild_t;
static_assert(sizeof(TBuild_t) == 0x0c);
static_assert(offsetof(TBuild_t, m_pTextureData) == 0x04);

typedef struct SharedTexture {
    TBuild_t m_TBuild;
    void* m_pEngineData;
    void* m_pRenderData;
    uint32_t unk_14[11];
} SharedTexture_t;
static_assert(sizeof(SharedTexture_t) == 0x40);
static_assert(offsetof(SharedTexture_t, m_pEngineData) == 0x0c);
static_assert(offsetof(SharedTexture_t, m_pRenderData) == 0x10);