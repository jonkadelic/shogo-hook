#pragma once

#include <stddef.h>
#include <stdint.h>

extern uintptr_t HOOK_OFFSET;

bool install_hook(uintptr_t target_addr, void* detour, void** original, char const* name);
