#pragma once

#include "lithtech/common.h"

typedef struct DRect {
    int32_t left;
    int32_t top;
    int32_t right;
    int32_t bottom;
} DRect_t;
static_assert(sizeof(DRect_t) == 0x10);
static_assert(offsetof(DRect_t, left) == 0x00);
static_assert(offsetof(DRect_t, top) == 0x04);
static_assert(offsetof(DRect_t, right) == 0x08);
static_assert(offsetof(DRect_t, bottom) == 0x0c);

typedef struct DVector {
    float x, y, z;
} DVector_t;
static_assert(sizeof(DVector_t) == 0x0c);
static_assert(offsetof(DVector_t, x) == 0x00);
static_assert(offsetof(DVector_t, y) == 0x04);
static_assert(offsetof(DVector_t, z) == 0x08);

typedef struct DVertex {
    float x, y, z, w;
} DVertex_t;
static_assert(sizeof(DVertex_t) == 0x10);
static_assert(offsetof(DVertex_t, x) == 0x00);
static_assert(offsetof(DVertex_t, y) == 0x04);
static_assert(offsetof(DVertex_t, z) == 0x08);
static_assert(offsetof(DVertex_t, w) == 0x0c);

typedef struct DRotation {
    DVector_t m_Vec;
    float m_Spin;
} DRotation_t;
static_assert(sizeof(DRotation_t) == 0x10);
static_assert(offsetof(DRotation_t, m_Vec) == 0x00);
static_assert(offsetof(DRotation_t, m_Spin) == 0x0c);

typedef struct DMatrix {
    float m[4][4];
} DMatrix_t;
static_assert(sizeof(DMatrix_t) == 0x40);
static_assert(offsetof(DMatrix_t, m) == 0x00);
