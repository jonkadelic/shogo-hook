#pragma once

#include "./common.h"
#include "./shapes.h"

typedef struct [[gnu::packed]] BlitRequest {
    void* m_pSurface;
    uint32_t m_BlitOptions;
    uint16_t m_TransparentColor;
    uint16_t unknown0;
    DRect_t* m_pSrcRect;
    DRect_t* m_pDestRect;
    uint16_t unknown1;
} BlitRequest_t;
static_assert(sizeof(BlitRequest_t) == 0x16);
static_assert(offsetof(BlitRequest_t, m_pSurface) == 0x00);
static_assert(offsetof(BlitRequest_t, m_BlitOptions) == 0x04);
static_assert(offsetof(BlitRequest_t, m_TransparentColor) == 0x08);
static_assert(offsetof(BlitRequest_t, m_pSrcRect) == 0x0C);
static_assert(offsetof(BlitRequest_t, m_pDestRect) == 0x10);
