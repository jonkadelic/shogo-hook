#include "./hook.h"

#include <stdio.h>

#include <MinHook.h>

uintptr_t HOOK_OFFSET = 0;

bool install_hook(uintptr_t target_addr, void* detour, void** out_original, char const* name) {
    printf("Address of %s: 0x%p\n", name, (void*) (HOOK_OFFSET + target_addr));

    MH_STATUS status = MH_CreateHook(
        (void*) (HOOK_OFFSET + target_addr),
        detour,
        out_original
    );

    if (status != MH_OK) {
        char msg[256];
        sprintf_s(msg, sizeof(msg), "Failed to create hook for %s: %d\n", name, status);
        printf("%s", msg);
        return false;
    }

    status = MH_EnableHook((void*) (HOOK_OFFSET + target_addr));
    if (status != MH_OK) {
        char msg[256];
        sprintf_s(msg, sizeof(msg), "Failed to enable hook for %s: %d\n", name, status);
        printf("%s", msg);
        return false;
    }

    return true;
}