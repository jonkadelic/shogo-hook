#pragma once

#include "./common.h"

#include "lithtech/link.h"
#include "lithtech/texture.h"

typedef struct SpriteEntry {
    SharedTexture_t* m_pTex;
} SpriteEntry_t;
static_assert(sizeof(SpriteEntry_t) == 0x04);
static_assert(offsetof(SpriteEntry_t, m_pTex) == 0x00);

typedef struct SpriteAnim {
    char m_sName[32];
    SpriteEntry_t* m_Frames;
    uint32_t m_nFrames;
    uint32_t m_MsAnimLength;
    uint32_t m_MsFrameRate;
    DBOOL m_bKeyed;
    uint32_t m_ColourKey;
    DBOOL m_bTranslucent;
} SpriteAnim_t;
static_assert(sizeof(SpriteAnim_t) == 0x3c);
static_assert(offsetof(SpriteAnim_t, m_sName) == 0x00);
static_assert(offsetof(SpriteAnim_t, m_Frames) == 0x20);
static_assert(offsetof(SpriteAnim_t, m_nFrames) == 0x24);
static_assert(offsetof(SpriteAnim_t, m_MsAnimLength) == 0x28);
static_assert(offsetof(SpriteAnim_t, m_MsFrameRate) == 0x2c);
static_assert(offsetof(SpriteAnim_t, m_bKeyed) == 0x30);
static_assert(offsetof(SpriteAnim_t, m_ColourKey) == 0x34);
static_assert(offsetof(SpriteAnim_t, m_bTranslucent) == 0x38);

typedef struct Sprite {
    DLink_t m_Link;
    SpriteAnim_t* m_Anims;
    uint32_t m_nAnims;
} Sprite_t;
static_assert(sizeof(Sprite_t) == 0x14);
static_assert(offsetof(Sprite_t, m_Link) == 0x00);
static_assert(offsetof(Sprite_t, m_Anims) == 0x0c);
static_assert(offsetof(Sprite_t, m_nAnims) == 0x10);

typedef struct SpriteTracker {
    Sprite_t* m_pSprite;
    SpriteAnim_t* m_pCurAnim;
    SpriteEntry_t* m_pCurFrame;
    uint32_t m_MsCurTime;
    uint32_t m_Flags;
} SpriteTracker_t;
static_assert(sizeof(SpriteTracker_t) == 0x14);
static_assert(offsetof(SpriteTracker_t, m_pSprite) == 0x00);
static_assert(offsetof(SpriteTracker_t, m_pCurAnim) == 0x04);
static_assert(offsetof(SpriteTracker_t, m_pCurFrame) == 0x08);
static_assert(offsetof(SpriteTracker_t, m_MsCurTime) == 0x0c);
static_assert(offsetof(SpriteTracker_t, m_Flags) == 0x10);