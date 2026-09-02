// adapted from https://github.com/Five-Damned-Dollarz/LithTech-1-Renderer/blob/main/source/worldbsp.d
#pragma once

#include <stdint.h>

#include "./shapes.h"
#include "./texture.h"
#include "./object.h"

typedef struct WorldData WorldData_t;
typedef struct WorldBsp WorldBsp_t;
typedef struct ObjectList ObjectList_t;
typedef struct WorldPoly WorldPoly_t;
typedef struct Leaf Leaf_t;
typedef struct Node Node_t;
typedef struct Surface Surface_t;
typedef struct MainWorld MainWorld_t;

typedef struct WorldData {
    WorldBsp_t* m_pWorldBsp;
    WorldBsp_t* m_pValidBsp;
    void* unk_08[2];
    WorldData_t* unk_10;
} WorldData_t;
static_assert(sizeof(WorldData_t) == 0x14);
static_assert(offsetof(WorldData_t, m_pWorldBsp) == 0x00);
static_assert(offsetof(WorldData_t, m_pValidBsp) == 0x04);

typedef struct Plane {
    DVector_t m_Vec;
    float m_Dist;
} Plane_t;
static_assert(sizeof(Plane_t) == 0x10);
static_assert(offsetof(Plane_t, m_Vec) == 0x00);
static_assert(offsetof(Plane_t, m_Dist) == 0x0c);

typedef enum NodeFlags: uint8_t {
    NodeFlags_Unknown = 0x8,
} NodeFlags_t;

typedef struct Node {
    NodeFlags_t m_Flags;
    WorldPoly_t* m_pPoly;
    Plane_t* m_pPlanes;
    int32_t m_Side;
    Leaf_t* m_pViewerLeaf;
    WorldBsp_t* m_pBsp;
    DVector_t m_vCenter;
    float m_fRadius;
    ObjectList_t* m_pObjects;
    Node_t* m_Sides[2];
} Node_t;
static_assert(sizeof(Node_t) == 0x34);
static_assert(offsetof(Node_t, m_Flags) == 0x00);
static_assert(offsetof(Node_t, m_pPoly) == 0x04);
static_assert(offsetof(Node_t, m_pPlanes) == 0x08);
static_assert(offsetof(Node_t, m_Side) == 0x0c);
static_assert(offsetof(Node_t, m_pViewerLeaf) == 0x10);
static_assert(offsetof(Node_t, m_pBsp) == 0x14);
static_assert(offsetof(Node_t, m_vCenter) == 0x18);
static_assert(offsetof(Node_t, m_fRadius) == 0x24);
static_assert(offsetof(Node_t, m_pObjects) == 0x28);
static_assert(offsetof(Node_t, m_Sides) == 0x2c);

typedef enum SurfaceFlags: uint32_t {
    SurfaceFlags_Solid = 0x1,
    SurfaceFlags_NonExistent = 0x2,
    SurfaceFlags_Invisible = 0x4,
    SurfaceFlags_Transparent = 0x8,
    SurfaceFlags_Sky = 0x10,
    SurfaceFlags_Bright = 0x20,
    SurfaceFlags_GouraudShade = 0x40,
    SurfaceFlags_LightMap = 0x80,
    SurfaceFlags_NoSubDiv = 0x200,
    SurfaceFlags_Hullmaker = 0x400,
    SurfaceFlags_AlwaysLightMap = 0x800,
    SurfaceFlags_DirectionalLight = 0x1000,
    SurfaceFlags_Unknown = 0x8000,
} SurfaceFlags_t;

typedef struct Surface {
    DVector_t m_OpaqueMap[6];
    SharedTexture_t* m_pTexture;
    Plane_t* m_pPlane;
    SurfaceFlags_t m_SurfaceFlags;
    uint16_t m_TextureFlags;
    uint16_t m_iTexture;
    uint32_t m_Index;
} Surface_t;
static_assert(sizeof(Surface_t) == 0x5c);
static_assert(offsetof(Surface_t, m_OpaqueMap) == 0x00);
static_assert(offsetof(Surface_t, m_pTexture) == 0x48);
static_assert(offsetof(Surface_t, m_pPlane) == 0x4c);
static_assert(offsetof(Surface_t, m_TextureFlags) == 0x54);
static_assert(offsetof(Surface_t, m_iTexture) == 0x56);
static_assert(offsetof(Surface_t, m_Index) == 0x58);

typedef struct LeafList {
    int16_t m_PortalID;
    uint16_t m_Length;
    uint8_t* m_pData;
} LeafList_t;
static_assert(sizeof(LeafList_t) == 0x08);
static_assert(offsetof(LeafList_t, m_PortalID) == 0x00);
static_assert(offsetof(LeafList_t, m_Length) == 0x02);
static_assert(offsetof(LeafList_t, m_pData) == 0x04);

typedef struct Leaf {
    float m_Vector[4];
    LeafList_t* m_pLeafList;
    void* unk_10;
    void* unk_14;
    void* unk_18;
    WorldPoly_t** m_ppPolies;
    uint32_t m_nPolies;
    void* unk_24;
    int32_t unk_28;
} Leaf_t;
static_assert(sizeof(Leaf_t) == 0x30);
static_assert(offsetof(Leaf_t, m_Vector) == 0x00);
static_assert(offsetof(Leaf_t, m_pLeafList) == 0x10);
static_assert(offsetof(Leaf_t, m_ppPolies) == 0x20);
static_assert(offsetof(Leaf_t, m_nPolies) == 0x24);

typedef struct PolyVertex {
    DVertex_t* m_pVertexData;
    float m_UV[2];
    float m_LightmapUV[2];
    uint8_t m_Color[4];
} PolyVertex_t;
static_assert(sizeof(PolyVertex_t) == 0x18);
static_assert(offsetof(PolyVertex_t, m_pVertexData) == 0x00);
static_assert(offsetof(PolyVertex_t, m_UV) == 0x04);
static_assert(offsetof(PolyVertex_t, m_LightmapUV) == 0x0c);
static_assert(offsetof(PolyVertex_t, m_Color) == 0x14);

typedef struct WorldPoly {
    DVector_t m_vCenter;
    float m_fRadius;
    Surface_t* m_pSurface;
    Plane_t* m_pPlane;
    WorldPoly_t* m_pNext;
    void* unk_1c;
    DVector_t m_PolygonList;
    void* m_pLightmapPage;
    uint8_t m_LightmapInfo[4];
    uint8_t* m_pLightmapData;
    uint16_t unk_38;
    uint16_t m_FrameCode;
    int32_t m_IsLightmapped;
    uint16_t m_nVertices;
    uint16_t m_VertexExtra;
    PolyVertex_t m_Vertices[];
} WorldPoly_t;
static_assert(sizeof(WorldPoly_t) == 0x44);
static_assert(offsetof(WorldPoly_t, m_vCenter) == 0x00);
static_assert(offsetof(WorldPoly_t, m_fRadius) == 0x0c);
static_assert(offsetof(WorldPoly_t, m_pSurface) == 0x10);
static_assert(offsetof(WorldPoly_t, m_pNext) == 0x18);
static_assert(offsetof(WorldPoly_t, m_PolygonList) == 0x20);
static_assert(offsetof(WorldPoly_t, m_pLightmapPage) == 0x2c);
static_assert(offsetof(WorldPoly_t, m_LightmapInfo) == 0x30);
static_assert(offsetof(WorldPoly_t, m_pLightmapData) == 0x34);
static_assert(offsetof(WorldPoly_t, m_FrameCode) == 0x3a);
static_assert(offsetof(WorldPoly_t, m_IsLightmapped) == 0x3c);
static_assert(offsetof(WorldPoly_t, m_nVertices) == 0x40);
static_assert(offsetof(WorldPoly_t, m_VertexExtra) == 0x42);

typedef struct PortalUnknown {
    void* unk_00;
    void* unk_04;
    int32_t unk_08[2];
    uint8_t unk_10[16];
} PortalUnknown_t;
static_assert(sizeof(PortalUnknown_t) == 0x20);

typedef struct Portal {
    PortalUnknown_t* unk_00;
    int32_t unk_04;
    int32_t m_Index;
    DVector_t m_Position;
    DVector_t m_Dimensions;
} Portal_t;
static_assert(sizeof(Portal_t) == 0x24);
static_assert(offsetof(Portal_t, m_Index) == 0x08);
static_assert(offsetof(Portal_t, m_Position) == 0x0c);
static_assert(offsetof(Portal_t, m_Dimensions) == 0x18);

typedef struct PBlockContents {
    uint8_t m_VertIndex;
    uint8_t unk_01;
    uint8_t m_Content[4];
} PBlockContents_t;
static_assert(sizeof(PBlockContents_t) == 0x06);
static_assert(offsetof(PBlockContents_t, m_VertIndex) == 0x00);
static_assert(offsetof(PBlockContents_t, m_Content) == 0x02);

typedef struct PBlock {
    int16_t m_Size;
    int16_t unk_02;
    PBlockContents_t* m_pContents;
} PBlock_t;
static_assert(sizeof(PBlock_t) == 0x08);
static_assert(offsetof(PBlock_t, m_Size) == 0x00);
static_assert(offsetof(PBlock_t, m_pContents) == 0x04);

typedef struct PBlockTable {
    DVector_t m_Vec1;
    DVector_t m_Vec2;
    int32_t unk_18;
    int32_t unk_1c;
    int32_t m_PBlockCount;
    int32_t unk_24;
    int32_t m_PBlockCountB;
    PBlock_t* m_pPBlockArray;
} PBlockTable_t;
static_assert(sizeof(PBlockTable_t) == 0x30);
static_assert(offsetof(PBlockTable_t, m_Vec1) == 0x00);
static_assert(offsetof(PBlockTable_t, m_Vec2) == 0x0c);
static_assert(offsetof(PBlockTable_t, m_PBlockCount) == 0x20);
static_assert(offsetof(PBlockTable_t, m_PBlockCountB) == 0x28);
static_assert(offsetof(PBlockTable_t, m_pPBlockArray) == 0x2c);

typedef struct WorldBsp {
    uint32_t m_MemoryUse;
    void* m_pNextSection;
    Plane_t* m_Planes;
    uint32_t m_nPlanes;
    Node_t* m_Nodes;
    uint32_t m_nNodes;
    ObjectList_t* m_WorldModels;
    uint32_t m_nWorldModels;
    Surface_t* m_Surfaces;
    uint32_t m_nSurfaces;
    LeafList_t* m_LeafLists;
    uint32_t m_nLeafLists;
    Leaf_t* m_Leaves;
    uint32_t m_nLeaves;
    int16_t* unk_38;
    uint32_t unk_3c;
    uint32_t unk_40;
    Node_t* m_RootNode;
    ObjectList_t* m_WorldModelRoot;
    WorldPoly_t** m_Polies;
    uint32_t m_nPolies;
    DVertex_t* m_Points;
    uint32_t m_nPoints;
    Portal_t* m_Portals;
    uint32_t m_nPortals;
    char* m_TextureNameData;
    char** m_TextureNames;
    uint32_t m_nTextureNames;
    uint32_t unk_64[26];
    DObject_t* m_pOwner;
    void* unk_dc;
    DVector_t m_MinBox;
    DVector_t m_MaxBox;
    DVector_t m_MinBoxPlus;
    DVector_t m_MaxBoxPlus;
    uint32_t m_WorldInfoFlags;
    uint32_t unk_114;
    float unk_118;
    DVector_t unk_11c;
    uint8_t* m_pLightmapData;
    uint32_t m_WorldFlags;
    void* unk_130;
    uint8_t* m_pLeafListContents;
    PBlockTable_t m_PBlockTable;
} WorldBsp_t;
static_assert(sizeof(WorldBsp_t) == 0x168);
static_assert(offsetof(WorldBsp_t, m_MemoryUse) == 0x00);
static_assert(offsetof(WorldBsp_t, m_pNextSection) == 0x04);
static_assert(offsetof(WorldBsp_t, m_Planes) == 0x08);
static_assert(offsetof(WorldBsp_t, m_nPlanes) == 0x0c);
static_assert(offsetof(WorldBsp_t, m_Nodes) == 0x10);
static_assert(offsetof(WorldBsp_t, m_nNodes) == 0x14);
static_assert(offsetof(WorldBsp_t, m_WorldModels) == 0x18);
static_assert(offsetof(WorldBsp_t, m_nWorldModels) == 0x1c);
static_assert(offsetof(WorldBsp_t, m_Surfaces) == 0x20);
static_assert(offsetof(WorldBsp_t, m_nSurfaces) == 0x24);
static_assert(offsetof(WorldBsp_t, m_LeafLists) == 0x28);
static_assert(offsetof(WorldBsp_t, m_nLeafLists) == 0x2c);
static_assert(offsetof(WorldBsp_t, m_Leaves) == 0x30);
static_assert(offsetof(WorldBsp_t, m_nLeaves) == 0x34);
static_assert(offsetof(WorldBsp_t, m_RootNode) == 0x44);
static_assert(offsetof(WorldBsp_t, m_MinBox) == 0xe0);
static_assert(offsetof(WorldBsp_t, m_MaxBox) == 0xec);
static_assert(offsetof(WorldBsp_t, m_pLightmapData) == 0x128);
static_assert(offsetof(WorldBsp_t, m_WorldFlags) == 0x12c);
static_assert(offsetof(WorldBsp_t, m_WorldModelRoot) == 0x48);
static_assert(offsetof(WorldBsp_t, m_Polies) == 0x4c);
static_assert(offsetof(WorldBsp_t, m_nPolies) == 0x50);
static_assert(offsetof(WorldBsp_t, m_Points) == 0x54);
static_assert(offsetof(WorldBsp_t, m_nPoints) == 0x58);
static_assert(offsetof(WorldBsp_t, m_Portals) == 0x5c);
static_assert(offsetof(WorldBsp_t, m_nPortals) == 0x60);
static_assert(offsetof(WorldBsp_t, m_TextureNameData) == 0x64);
static_assert(offsetof(WorldBsp_t, m_TextureNames) == 0x68);
static_assert(offsetof(WorldBsp_t, m_nTextureNames) == 0x6c);
static_assert(offsetof(WorldBsp_t, m_pOwner) == 0xd8);
static_assert(offsetof(WorldBsp_t, m_MinBoxPlus) == 0xf8);
static_assert(offsetof(WorldBsp_t, m_MaxBoxPlus) == 0x104);
static_assert(offsetof(WorldBsp_t, m_WorldInfoFlags) == 0x110);
static_assert(offsetof(WorldBsp_t, m_pLightmapData) == 0x128);
static_assert(offsetof(WorldBsp_t, m_WorldFlags) == 0x12c);
static_assert(offsetof(WorldBsp_t, m_pLeafListContents) == 0x134);
static_assert(offsetof(WorldBsp_t, m_PBlockTable) == 0x138);

typedef struct MainWorld {
    uint32_t m_MemoryUsed;
    WorldBsp_t* m_pWorldBsp;
    uint16_t unk_08[2];
    int32_t unk_0c;
    int32_t unk_10[4];
    DVector_t unk_20;
    void* unk_2c;
    uint32_t unk_30;
    int32_t unk_34[6];
    void* unk_4c;
    DVector_t unk_50[2];
    DVector_t unk_68;
    DVector_t m_ExtentsMin;
    DVector_t m_ExtentsMax;
    DVector_t m_ExtentsNormal;
    int32_t unk_98;
    void* unk_9c;
    WorldData_t** m_pWorldModels;
    uint32_t m_WorldModelCount;
    int32_t unk_a8;
    MainWorld_t* unk_ac;
    void* unk_b0;
    int32_t unk_b4[5];
    MainWorld_t* unk_c8;
} MainWorld_t;
static_assert(sizeof(MainWorld_t) == 0xcc);
static_assert(offsetof(MainWorld_t, m_pWorldBsp) == 0x04);
static_assert(offsetof(MainWorld_t, m_ExtentsMin) == 0x74);
static_assert(offsetof(MainWorld_t, m_ExtentsMax) == 0x80);
static_assert(offsetof(MainWorld_t, m_ExtentsNormal) == 0x8c);
static_assert(offsetof(MainWorld_t, m_pWorldModels) == 0xa0);
static_assert(offsetof(MainWorld_t, m_WorldModelCount) == 0xa4);
