// sourced from https://github.com/Five-Damned-Dollarz/LithTech-1-Renderer/blob/main/source/object/worldmodel.d
#pragma once

#include "lithtech/object/object.h"

typedef struct WorldData WorldData_t;

typedef struct WorldModelInstance {
    DObject_t base;
    WorldData_t* m_pWorldData;
    DMatrix_t m_Transform;
    DMatrix_t m_BackTransform;
} WorldModelInstance_t;
static_assert(sizeof(WorldModelInstance_t) == 0x1ac);
static_assert(offsetof(WorldModelInstance_t, base) == 0x000);
static_assert(offsetof(WorldModelInstance_t, m_pWorldData) == 0x128);
static_assert(offsetof(WorldModelInstance_t, m_Transform) == 0x12c);
static_assert(offsetof(WorldModelInstance_t, m_BackTransform) == 0x16c);
