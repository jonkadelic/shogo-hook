#pragma once

#include "./common.h"
#include "./shapes.h"
#include "./sprite.h"
#include "./link.h"

typedef enum ObjectType: uint8_t {
    ObjectType_Normal = 0, // Invisible object
    ObjectType_Model = 1, // Model object
    ObjectType_WorldModel = 2, // World model object
    ObjectType_Sprite = 3, // Sprite
    ObjectType_Light = 4, // Dynamic light
    ObjectType_Camera = 5, // Camera
    ObjectType_ParticleSystem = 6, // Particle system
    ObjectType_PolyGrid = 7, // Poly grid
    ObjectType_LineSystem = 8, // Line system
    ObjectType_Container = 9, // Container
    NumObjectTypes,
} ObjectType_t;

typedef struct DAttachment {

} DAttachment_t;

typedef struct DObject {
    void* unk_0[10];
    uint32_t m_Flags;
    uint32_t m_UserFlags;
    uint8_t m_ColorR;
    uint8_t m_ColorG;
    uint8_t m_ColorB;
    uint8_t m_ColorA;
    DAttachment_t* m_Attachments;
    DVector_t m_Pos;
    DRotation_t m_Rotation;
    DVector_t m_Scale;
    float unk_60;
    float unk_64;
    uint16_t m_ObjectID;
    uint16_t m_SerializeID;
    uint16_t unk_6c;
    ObjectType_t m_ObjectType;
    uint8_t unk_6f;
    uint32_t unk_70[7];
    DVector_t unk_8c;
    uint32_t unk_98[2];
    DVector_t m_MinBounds;
    DVector_t m_MaxBounds;
    DVector_t m_HalfSize;
} DObject_t;
static_assert(sizeof(DObject_t) == 0xc4);
static_assert(offsetof(DObject_t, m_Flags) == 0x28);
static_assert(offsetof(DObject_t, m_UserFlags) == 0x2c);
static_assert(offsetof(DObject_t, m_ColorR) == 0x30);
static_assert(offsetof(DObject_t, m_ColorG) == 0x31);
static_assert(offsetof(DObject_t, m_ColorB) == 0x32);
static_assert(offsetof(DObject_t, m_ColorA) == 0x33);
static_assert(offsetof(DObject_t, m_Attachments) == 0x34);
static_assert(offsetof(DObject_t, m_Pos) == 0x38);
static_assert(offsetof(DObject_t, m_Rotation) == 0x44);
static_assert(offsetof(DObject_t, m_Scale) == 0x54);
static_assert(offsetof(DObject_t, m_ObjectID) == 0x68);
static_assert(offsetof(DObject_t, m_SerializeID) == 0x6a);
static_assert(offsetof(DObject_t, m_ObjectType) == 0x6e);
static_assert(offsetof(DObject_t, unk_8c) == 0x8c);
static_assert(offsetof(DObject_t, m_MinBounds) == 0xa0);
static_assert(offsetof(DObject_t, m_MaxBounds) == 0xac);
static_assert(offsetof(DObject_t, m_HalfSize) == 0xb8);

typedef struct PolyGrid {
    DObject_t base;
    uint32_t unk_00c4[25];
    uint8_t* m_pData;
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
    uint32_t m_Width;
    uint32_t m_Height;
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
