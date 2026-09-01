#pragma once

#define DEBUG

#define OBJECT_ZERO_INIT(self) (*self = (typeof(*self)) { 0 })

#define EXTRACT_RGB565(val, _r, _g, _b) *(_r) = ((val) & 0xF800) >> 11; *(_g) = ((val ) & 0x07E0) >> 5; *(_b) = ((val) & 0x001F)
#define RGB565_TO_RGBA8888(r5, g6, b5, a8) (((a8) << 24) | ((((b5) << 3) | ((b5) >> 2)) << 16) | ((((g6) << 2) | ((g6) >> 4)) << 8) | (((r5) << 3) | ((r5) >> 2)))