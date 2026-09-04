#pragma once

#include "lithtech/object/object.h"

typedef struct ModelVertex {
    DVector_t m_Position;
    float m_UV[2];
    int8_t m_Normal[3];
    uint8_t m_NodeIndex;
    uint16_t m_DeformReplacements;
} ModelVertex_t;

typedef struct ModelFace {
    uint16_t m_Vertices[3];
    uint8_t m_Normals[3];
    uint8_t unk_09;
} ModelFace_t;

typedef struct ModelNode {
    char* m_pName;
    DVector_t m_BoundsMin;
    DVector_t m_BoundsMax;
    void* unk_1c;
    uint16_t* m_pDeformVertices;
    uint32_t m_nDeformVertices;
    void* unk_28;
    uint16_t m_Index;
    uint16_t m_Flags;
    struct ModelNode* m_ChildNodes;
    uint32_t m_nChildNodes;
} ModelNode_t;
static_assert(sizeof(ModelNode_t) == 0x38);

typedef struct ModelData {
    DLink_t m_Link;
    char* m_pFilename;
    void* unk_010[2];
    DVector_t m_BoundsMin;
    DVector_t m_BoundsMax;
    float m_fRadius;
    void* unk_048[3];
    uint32_t unk_54;
    void* unk_58[12];
    uint32_t m_Flags;
    ModelNode_t* unk_078;
    uint32_t unk_07c;
    ModelNode_t** m_pNodes;
    uint32_t m_nNodes;
    ModelNode_t** unk_088;
    uint32_t unk_08c[2];
    uint32_t m_nVerticesTotal;
    ModelVertex_t* m_pVertices;
    uint32_t m_nVerticesBase;
    uint32_t m_nFaces;
    ModelFace_t* m_pFaces;
    float* m_pUVs;
    void* unk_0ac;
    uint32_t m_nLods;
    float m_fLodStartDist;
    float m_fLodInterval;
    float m_fLodMaxDist;
    float unk_c0;
    uint32_t unk_c4;
    DMatrix_t* m_pNodeMatrices;
    uint32_t m_nNodeMatrices;
} ModelData_t;

typedef struct ModelInstance {
    DObject_t base;
    SharedTexture_t* m_pSkin;
    uint32_t unk_12c;
    SharedTexture_t* unk_130; // type as per r_SetModelTextureInfo
    uint32_t unk_134[3];
    ModelData_t* m_pModelData;
    uint32_t unk_144;
    // odds are there are more fields after this, but we don't *currently* care
} ModelInstance_t;
