#include "./logger.h"

#include <windows.h>
#include <stdio.h>

#include <SDL3/SDL.h>

static char const* const LOG_LEVEL_STRS[] = {
    [LOG_LEVEL__DEBUG] = "DEBUG",
    [LOG_LEVEL__INFO] = "INFO",
    [LOG_LEVEL__WARNING] = "WARNING",
    [LOG_LEVEL__ERROR] = "ERROR",
    [LOG_LEVEL__FATAL] = "FATAL",
};

// not thread-safe
static FILE* LOG_FILE = nullptr;

bool logger__init(void) {
    char filename[MAX_PATH + 1];

    LOG_FILE = fopen("latest.log", "wb");
    if (LOG_FILE == nullptr) {
        goto err;
    }

    return true;

err:
    logger__cleanup();
    return false;
}

void logger__cleanup() {
    fclose(LOG_FILE);
    LOG_FILE = nullptr;
}

[[gnu::format(printf, 2, 3)]]
void logger__log(log_level_t level, char const* fmt, ...) {
    static char msg_buf[1024];

    uint64_t ticks = SDL_GetTicks();
    
    va_list vargs;
    va_start(vargs, fmt);
    vsnprintf(msg_buf, sizeof(msg_buf), fmt, vargs);
    va_end(vargs);

    if (level >= LOG_LEVEL__INFO) {
        printf("[%0.3f] (%s) %s\n", (float) ticks / 1000.0f, LOG_LEVEL_STRS[level], msg_buf);
    }

    if (LOG_FILE != nullptr) {
        fprintf(LOG_FILE, "[%0.3f] (%s) %s\n", (float) ticks / 1000.0f, LOG_LEVEL_STRS[level], msg_buf);
        if (fflush(LOG_FILE) != 0) {
            fclose(LOG_FILE);
            fopen("latest.log", "wb");
            fprintf(LOG_FILE, "[%0.3f] (%s) %s\n", (float) ticks / 1000.0f, LOG_LEVEL_STRS[level], msg_buf);
        }
    }

    if (level == LOG_LEVEL__FATAL) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", msg_buf, nullptr);
    }
}
