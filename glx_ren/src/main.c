
#include <stdio.h>
#include <windows.h>

#include <SDL3/SDL.h>
#include <lithtech/lithtech.h>

#include "debug.h"
#include "renderer.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    switch (reason) {
        case DLL_PROCESS_ATTACH: {
            DisableThreadLibraryCalls(hModule);
            if (!SDL_Init(SDL_INIT_VIDEO)) {
                return FALSE;
            }
        } break;
        case DLL_PROCESS_DETACH: {
            SDL_Quit();
        } break;
    }

    return TRUE;
}

void RenderDLLSetup(RenderStruct_t* pStruct) {
    pStruct->Init = r_Init;
    pStruct->Term = r_Term;
    pStruct->BindTexture = r_BindTexture;
    pStruct->UnbindTexture = r_UnbindTexture;
    pStruct->SetMasterPalette = r_SetMasterPalette;
    pStruct->CreateContext = r_CreateContext;
    pStruct->DeleteContext = r_DeleteContext;
    pStruct->Clear = r_Clear;
    pStruct->Start3D = r_Start3D;
    pStruct->End3D = r_End3D;
    pStruct->IsIn3D = r_IsIn3D;
    pStruct->StartOptimized2D = r_StartOptimized2D;
    pStruct->EndOptimized2D = r_EndOptimized2D;
    pStruct->IsInOptimized2D = r_IsInOptimized2D;
    pStruct->OptimizeSurface = r_OptimizeSurface;
    pStruct->UnoptimizeSurface = r_UnoptimizeSurface;
    pStruct->RenderScene = r_RenderScene;
    pStruct->RenderCommand = r_RenderCommand;
    pStruct->GetHook = r_GetHook;
    pStruct->SwapBuffers = r_SwapBuffers;
    pStruct->GetInfoFlags = r_GetInfoFlags;
    pStruct->GetBufferFormat = r_GetBufferFormat;
    pStruct->CreateSurface = r_CreateSurface;
    pStruct->DeleteSurface = r_DeleteSurface;
    pStruct->GetSurfaceInfo = r_GetSurfaceInfo;
    pStruct->LockSurface = r_LockSurface;
    pStruct->UnlockSurface = r_UnlockSurface;
    pStruct->LockScreen = r_LockScreen;
    pStruct->UnlockScreen = r_UnlockScreen;
    pStruct->MakeScreenShot = r_MakeScreenShot;
    pStruct->QueryDeletePalette = r_QueryDeletePalette;
    pStruct->SetSoftSky = r_SetSoftSky;
    pStruct->ReadConsoleVariables = r_ReadConsoleVariables;
    pStruct->BlitToScreen = r_BlitToScreen;
}

RMode_t* GetSupportedModes(void) {
    size_t num_rmodes = 0;
    RMode_t* rmodes = nullptr;

    int num_displays = 0;
    SDL_DisplayID* displays = nullptr;

    int num_display_modes = 0;
    SDL_DisplayMode** display_modes = nullptr;

    size_t largest_display_size = 0;
    SDL_DisplayID largest_display;

    displays = SDL_GetDisplays(&num_displays);
    if (displays == nullptr) {
        goto err;
    }

    for (int i = 0; i < num_displays; i++) {
        display_modes = SDL_GetFullscreenDisplayModes(displays[i], &num_display_modes);
        if (display_modes == nullptr) {
            goto err;
        }

        for (int j = 0; j < num_display_modes; j++) {
            num_rmodes++;
            rmodes = realloc(rmodes, sizeof(RMode_t) * num_rmodes);
            DEBUG_ASSERT(rmodes != nullptr);

            SDL_DisplayMode* mode = display_modes[j];
            RMode_t* rmode = &rmodes[num_rmodes - 1];
            memset(rmode, 0, sizeof(RMode_t));

            rmode->m_bHardware = 1;
            snprintf(rmode->m_RenderDLL, sizeof(rmode->m_RenderDLL), "gl3.ren");
            snprintf(rmode->m_InternalName, sizeof(rmode->m_InternalName), "sdl:%u", displays[i]);
            snprintf(rmode->m_Description, sizeof(rmode->m_Description), "%s", SDL_GetDisplayName(displays[i]));
            rmode->m_Width = mode->w;
            rmode->m_Height = mode->h;
            rmode->m_BitDepth = SDL_BITSPERPIXEL(mode->format);

            if (mode->w * mode->h > largest_display_size) {
                largest_display = displays[i];
                largest_display_size = mode->w * mode->h;
            }
        }

        SDL_free(display_modes);
    }

    SDL_DisplayMode const* current_display_mode = SDL_GetCurrentDisplayMode(largest_display);
    if (current_display_mode == nullptr) {
        goto err;
    }

    display_modes = SDL_GetFullscreenDisplayModes(largest_display, &num_display_modes);
    if (display_modes == nullptr) {
        goto err;
    }

    for (int i = 0; i < num_display_modes; i++) {
        SDL_DisplayMode* mode = display_modes[i];

        if (current_display_mode->format != mode->format) {
            continue;
        }

        num_rmodes++;
        rmodes = realloc(rmodes, sizeof(RMode_t) * num_rmodes);
        DEBUG_ASSERT(rmodes != nullptr);

        RMode_t* rmode = &rmodes[num_rmodes - 1];
        memset(rmode, 0, sizeof(RMode_t));
        snprintf(rmode->m_RenderDLL, sizeof(rmode->m_RenderDLL), "gl3.ren");
        snprintf(rmode->m_InternalName, sizeof(rmode->m_InternalName), "windowed");
        snprintf(rmode->m_Description, sizeof(rmode->m_Description), "Windowed");
        rmode->m_Width = mode->w;
        rmode->m_Height = mode->h;
        rmode->m_BitDepth = SDL_BITSPERPIXEL(mode->format);
    }

    SDL_free(display_modes);

    for (size_t i = 0; i < num_rmodes - 1; i++) {
        rmodes[i].m_pNext = &rmodes[i + 1];
    }

    SDL_free(displays);

    return rmodes;

err:
    free(rmodes);
    SDL_free(displays);
    SDL_free(display_modes);
    return nullptr;
}

void FreeModeList(RMode_t* modes) {
    free(modes);
}