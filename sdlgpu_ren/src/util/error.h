#pragma once

#include <SDL3/SDL_log.h>

#include "util/debug.h"

void error__display_fatal(SDL_LogCategory log_category, char const* fmt, ...);
