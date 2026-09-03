#pragma once

#include "lithtech/common.h"
#include "lithtech/shapes.h"
#include "lithtech/sprite.h"
#include "lithtech/link.h"

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
    uint32_t unk_00c4[25];
} DObject_t;
static_assert(sizeof(DObject_t) == 0x128);
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

typedef struct Node Node_t;

typedef struct DObjectList {
    struct DObjectList* m_pPrev;
    struct DObjectList* m_pNext;
    DObject_t* m_pObject;
    uint32_t unk_0c[3];
} DObjectList_t;
