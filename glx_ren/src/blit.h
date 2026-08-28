#pragma once

#include <stddef.h>
#include <stdint.h>

#include <lithtech/lithtech.h>

typedef struct surface surface_t;

void blit__blit_to_screen(BlitRequest_t* request);