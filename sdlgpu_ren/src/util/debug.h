#pragma once

#include <windows.h>
#include <stdio.h>

#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT_FUN() printf("DEBUG: called %s\n", __FUNCTION__)//; DEBUG_SLEEP_FOR(100)
#define DEBUG_PRINT(str) printf("DEBUG: %s\n", str)
#define DEBUG_HANG() do { } while (1)
#define DEBUG_ASSERT(expression) do { while (!(expression)) { printf("Assert failed: %s:%d", __FUNCTION__, __LINE__); } } while (0)
#define DEBUG_HANG_MSG(msg, ...) printf("DEBUG HANG: " msg __VA_OPT__(,) __VA_ARGS__); DEBUG_HANG()
#define DEBUG_PRINT_PTR(rows, ptr) for (size_t row = 0; row < rows; row++) { for (size_t col = 0; col < 16; col++) { printf("%02X ", ((uint8_t*) ptr)[row * 16 + col]); } printf("\n"); }
#define DEBUG_SLEEP() Sleep(500)
#define DEBUG_SLEEP_FOR(time) Sleep(time)
#else
#define DEBUG_PRINT_FUN()
#define DEBUG_PRINT(str)
#define DEBUG_HANG()
#define DEBUG_ASSERT(expression)
#define DEBUG_HANG_MSG(msg, ...)
#define DEBUG_PRINT_PTR(rows, ptr)
#define DEBUG_SLEEP()
#define DEBUG_SLEEP_FOR(time)
#endif