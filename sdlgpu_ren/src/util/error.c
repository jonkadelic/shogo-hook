#include "./error.h"

#include <stdio.h>
#include <stdarg.h>

#include "render/renderer.h"
#include "util/debug.h"

void error__display_fatal(SDL_LogCategory log_category, char const* fmt, ...) {
    va_list list;
    va_start(list, fmt);

#ifdef DEBUG
    SDL_LogMessageV(log_category, SDL_LOG_PRIORITY_CRITICAL, fmt, list);
    DEBUG_HANG();
#else
    auto r = renderer__get();
    static char MESSAGE[1000];
    vsnprintf(MESSAGE, sizeof(MESSAGE), fmt, list);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SDL Error", MESSAGE, r->window);
#endif
}