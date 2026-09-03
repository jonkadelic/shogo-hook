#pragma once

#include "lithtech/object/object.h"

typedef struct PolyGrid {
    DObject_t base;
    int8_t* m_pData; // length = m_Width * m_Height
    uint16_t* m_pIndices;
    void* m_pSprite;
    SpriteTracker_t m_SpriteTracker;
    void* m_pEnvMap;
    float m_xPan;
    float m_yPan;
    float m_xScale;
    float m_yScale;
    uint32_t m_nTris; // = (m_Width - 1) * (m_Height - 1) * 2
    uint32_t m_nIndices; // = m_nTris * 3
    DLink_t m_LeafLinks;
    uint32_t m_Width;   // number of points along X axis
    uint32_t m_Height;  // number of points along Z axis
    float m_ColorTable[256][4];
} PolyGrid_t;
static_assert(sizeof(PolyGrid_t) == 0x1178);
static_assert(offsetof(PolyGrid_t, base) == 0x0000);
static_assert(offsetof(PolyGrid_t, m_pData) == 0x0128);
static_assert(offsetof(PolyGrid_t, m_pIndices) == 0x012c);
static_assert(offsetof(PolyGrid_t, m_pSprite) == 0x0130);
static_assert(offsetof(PolyGrid_t, m_SpriteTracker) == 0x0134);
static_assert(offsetof(PolyGrid_t, m_pEnvMap) == 0x0148);
static_assert(offsetof(PolyGrid_t, m_xPan) == 0x014c);
static_assert(offsetof(PolyGrid_t, m_yPan) == 0x0150);
static_assert(offsetof(PolyGrid_t, m_xScale) == 0x0154);
static_assert(offsetof(PolyGrid_t, m_yScale) == 0x0158);
static_assert(offsetof(PolyGrid_t, m_nTris) == 0x015c);
static_assert(offsetof(PolyGrid_t, m_nIndices) == 0x0160);
static_assert(offsetof(PolyGrid_t, m_LeafLinks) == 0x0164);
static_assert(offsetof(PolyGrid_t, m_Width) == 0x0170);
static_assert(offsetof(PolyGrid_t, m_Height) == 0x0174);
static_assert(offsetof(PolyGrid_t, m_ColorTable) == 0x0178);
