#pragma once

#include "lithtech/object/object.h"

typedef struct ModelAnimNode ModelAnimNode_t;
typedef struct ModelNode ModelNode_t;

typedef struct ModelAnimKeyframe {
    uint32_t m_TimeIndex;
    DVector_t m_BoundsMin;
    DVector_t m_BoundsMax;
    void* unk_1c;
    uint8_t unk_20;
    uint8_t pad_21[3];
    uint32_t pad_24[2];
} ModelAnimKeyframe_t;
static_assert(sizeof(ModelAnimKeyframe_t) == 0x2c);

typedef struct ModelNodeTransform {
    DVector_t m_Position;
    DRotation_t m_Rotation;
} ModelNodeTransform_t;

typedef struct ModelAnim {
    char* m_pName;
    uint32_t m_DurationMs;
    DVector_t m_Dims;
    void** unk_14;
    DVector_t m_BoundsMin;
    DVector_t m_BoundsMax;
    float m_fRadius;
    ModelAnimKeyframe_t* m_pKeyframes;
    uint32_t m_nKeyframes;
    void* unk_3c;
    ModelNode_t* unk_40;
    struct ModelAnim* unk_44;
    void* unk_48[2];
    DVector_t m_Scale;
    DVector_t m_Translation;
    ModelNodeTransform_t* m_pNodeTransforms;
    void* unk_68;
    ModelAnimNode_t* m_pRootNodes;
} ModelAnim_t;
static_assert(sizeof(ModelAnim_t) == 0x74); // determined through debugging

typedef struct ModelAnimNode {
    ModelNode_t* m_pNode;
    ModelAnim_t* m_pAnim;
    uint8_t* m_pCompressedVertices;
    void* unk_0c;
    DVector_t m_Scale;
    DVector_t m_Translation;
    ModelNodeTransform_t* m_pNodeTransforms;
    ModelAnimNode_t* unk_2c;
    ModelAnimNode_t* m_pChildren;
} ModelAnimNode_t;
static_assert(sizeof(ModelAnimNode_t) == 0x34);

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
    uint32_t unk_28;
    uint16_t m_Index;
    uint16_t m_Flags;
    struct ModelNode* m_pChildNodes;
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
    ModelNode_t** m_ppNodes;
    uint32_t m_nNodes;
    ModelNode_t** unk_088;
    uint32_t unk_08c[2];
    uint32_t m_nVerticesTotal;
    ModelVertex_t* m_pVertices;
    uint32_t m_nVerticesBase;
    uint32_t m_nFaces;
    ModelFace_t* m_pFaces;
    float* m_pUVs;
    void* m_pLods;
    uint32_t m_nLods;
    float m_fLodStartDist;
    float m_fLodInterval;
    float m_fLodMaxDist;
    float unk_c0;
    DMatrix_t* unk_c4;
    DMatrix_t* m_pNodeMatrices;
    uint32_t m_nNodeMatrices;
    char* m_pCommandString;
    void* unk_d4;
    float unk_d8;
    void* unk_dc[6];
    ModelAnim_t* m_pAnimations;
    uint32_t m_nAnimations;
    void* unk_fc;
} ModelData_t;

typedef struct ModelFrame {
    ModelAnim_t* m_pAnimation;
    ModelAnimKeyframe_t* m_pKeyframe;
    uint32_t m_AnimTimeMs;
    uint32_t m_FrameIndex;
} ModelFrame_t;

typedef struct ModelAnimTracker {
    uint32_t unk_00;
    SharedTexture_t* unk_04;
    uint32_t unk_08[3];
    ModelData_t* m_pModelData;
    uint32_t unk_18;
    uint32_t m_Flags;
    ModelFrame_t m_Frames[2];
    float m_fLerp;
} ModelAnimTracker_t;

typedef struct ModelInstance {
    DObject_t base;
    SharedTexture_t* m_pSkin;
    ModelAnimTracker_t m_Tracker;
    // odds are there are more fields after this, but we don't *currently* care
} ModelInstance_t;
static_assert(offsetof(ModelInstance_t, m_pSkin) == 0x128);
