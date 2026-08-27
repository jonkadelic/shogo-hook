#pragma once

#include "./common.h"

typedef struct DLink {
    struct DLink* m_pPrev;
    struct DLink* m_pNext;
    void* m_pData;
} DLink_t;
static_assert(sizeof(DLink_t) == 0x0c);
static_assert(offsetof(DLink_t, m_pPrev) == 0x00);
static_assert(offsetof(DLink_t, m_pNext) == 0x04);
static_assert(offsetof(DLink_t, m_pData) == 0x08);