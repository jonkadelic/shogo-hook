#pragma once

#include "./common.h"

typedef struct SpriteTracker {
    void* m_pSprite;
    void* m_pCurAnim;
    void* m_pCurFrame;
    uint32_t m_MsCurTime;
    uint32_t m_Flags;
} SpriteTracker_t;
static_assert(sizeof(SpriteTracker_t) == 0x14);
static_assert(offsetof(SpriteTracker_t, m_pSprite) == 0x00);
static_assert(offsetof(SpriteTracker_t, m_pCurAnim) == 0x04);
static_assert(offsetof(SpriteTracker_t, m_pCurFrame) == 0x08);
static_assert(offsetof(SpriteTracker_t, m_MsCurTime) == 0x0c);
static_assert(offsetof(SpriteTracker_t, m_Flags) == 0x10);