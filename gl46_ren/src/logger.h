#pragma once

#define LOG_DEBUG(fmt, ...) logger__log(LOG_LEVEL__DEBUG, (fmt) __VA_OPT__(,) __VA_ARGS__)
#define LOG_INFO(fmt, ...) logger__log(LOG_LEVEL__INFO, (fmt) __VA_OPT__(,) __VA_ARGS__)
#define LOG_WARNING(fmt, ...) logger__log(LOG_LEVEL__WARNING, (fmt) __VA_OPT__(,) __VA_ARGS__)
#define LOG_ERROR(fmt, ...) logger__log(LOG_LEVEL__ERROR, (fmt) __VA_OPT__(,) __VA_ARGS__)
#define LOG_FATAL(fmt, ...) logger__log(LOG_LEVEL__FATAL, (fmt) __VA_OPT__(,) __VA_ARGS__)

#define LOG_FUNC() LOG_DEBUG("Entered %s.", __FUNCTION__)

typedef enum log_level {
    LOG_LEVEL__DEBUG,
    LOG_LEVEL__INFO,
    LOG_LEVEL__WARNING,
    LOG_LEVEL__ERROR,
    LOG_LEVEL__FATAL,
} log_level_t;

bool logger__init(void);
void logger__cleanup(void);

[[gnu::format(printf, 2, 3)]]
void logger__log(log_level_t level, char const* fmt, ...);
