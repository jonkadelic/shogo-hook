#pragma once

#include "lithtech/common.h"
#include "lithtech/link.h"

typedef struct SharedTexture SharedTexture_t;

typedef struct DTXHeader {
    uint32_t m_ResType;
    int32_t m_Version;
    uint16_t m_BaseWidth;
    uint16_t m_BaseHeight;
    uint16_t m_nMipmaps;
    uint16_t m_bHasLights;
    uint32_t m_IFlags;
    uint32_t m_UserFlags;
    uint8_t m_TextureGroup;
    uint8_t m_nMipmapsToUse;
    uint8_t m_AlphaCutoff;
    uint8_t m_AlphaAverage;
    uint32_t unk_1c;
    uint32_t unk_20;
    uint8_t unk_24;
    uint8_t unk_25;
    uint16_t unk_26;
    uint8_t unk_28;
    uint8_t unk_29;
    uint16_t unk_2a;
} DTXHeader_t;

typedef struct DTXPalette {
    DLink_t m_Link;
    uint32_t unk_0c;
    uint32_t unk_10;
    uint32_t unk_14;
    uint32_t m_Colors[256];
} DTXPalette_t;

typedef struct TextureData {
    uint32_t m_ResHeader;
    DTXHeader_t m_Header;
    void* unk_0c;
    uint32_t unk_10;
    DTXPalette_t* m_pPalette;
    uint32_t unk_18;
    SharedTexture_t* m_pSharedTexture; // From whence it came..
    uint32_t m_Flags; 
    uint32_t m_Width;
    uint32_t m_Height;
    uint32_t m_PitchBytes;
    uint8_t* m_pDataBuffer;
    uint8_t* m_pAlphaBuffer;
} TextureData_t;
static_assert(sizeof(TextureData_t) == 0x5c);
static_assert(offsetof(TextureData_t, m_Width) == 0x48);
static_assert(offsetof(TextureData_t, m_Height) == 0x4c);

typedef struct TBuild {
    uint32_t unk_00;
    TextureData_t* m_pTextureData;
    uint32_t unk_08;
} TBuild_t;
static_assert(sizeof(TBuild_t) == 0x0c);
static_assert(offsetof(TBuild_t, m_pTextureData) == 0x04);

typedef struct FileIdentifier {
    void* m_pData;
    DLink_t m_Link;
    void* m_hFileTree;
    uint16_t m_FileId;
    uint16_t m_NameLen;
    uint8_t m_TypeCode;
    uint8_t m_Flags;
    char* m_Filename;
} FileIdentifier_t;

typedef struct SharedTexture {
    TBuild_t m_TBuild;
    void* m_pEngineData;
    void* m_pRenderData;
    DLink_t m_Link;
    FileIdentifier_t* m_pFile;
    uint32_t m_eTexType; // temp - replace with enum
    struct SharedTexture* m_pLinkedTexture[4];
    uint16_t m_RefCount;
    uint16_t unk_3a;
    uint16_t m_nWidth;
    uint16_t m_nHeight;
    uint16_t m_nBpp;
    void* unk_44;
} SharedTexture_t;
static_assert(sizeof(SharedTexture_t) == 0x48);
static_assert(offsetof(SharedTexture_t, m_TBuild) == 0x00);
static_assert(offsetof(SharedTexture_t, m_pEngineData) == 0x0c);
static_assert(offsetof(SharedTexture_t, m_pRenderData) == 0x10);
static_assert(offsetof(SharedTexture_t, m_Link) == 0x14);
static_assert(offsetof(SharedTexture_t, m_pFile) == 0x20);
static_assert(offsetof(SharedTexture_t, m_eTexType) == 0x24);
static_assert(offsetof(SharedTexture_t, m_pLinkedTexture) == 0x28);
static_assert(offsetof(SharedTexture_t, m_RefCount) == 0x38);
static_assert(offsetof(SharedTexture_t, m_nWidth) == 0x3c);
static_assert(offsetof(SharedTexture_t, m_nHeight) == 0x3e);
static_assert(offsetof(SharedTexture_t, m_nBpp) == 0x40);
