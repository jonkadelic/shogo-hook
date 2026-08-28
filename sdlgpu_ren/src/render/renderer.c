#include "./renderer.h"

#include <assert.h>
#include <windows.h>
#include <stdio.h>

#include <SDL3/SDL.h>

#include "render/object.h"
#include "util/error.h"
#include "util/util.h"

static bool init_window(SDL_Window* window, RMode_t const* rmode);

renderer_t* renderer__get(void) {
    static renderer_t RENDERER = { 0 };
    return &RENDERER;
}

bool renderer__init(renderer_t* self, void* hwnd, RMode_t const* mode) {
    ZERO_INIT_STRUCT(self);
    
    SDL_PropertiesID props = 0;

    if (hwnd == nullptr || mode == nullptr) {
        goto err;
    }

    // Init SDL3
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        error__display_fatal(SDL_LOG_CATEGORY_SYSTEM, "Failed to init SDL: %s", SDL_GetError());
        goto err;
    }

    // Init window properties
    props = SDL_CreateProperties();
    if (props == 0) {
        error__display_fatal(SDL_LOG_CATEGORY_SYSTEM, "Failed to create SDL window properties: %s", SDL_GetError());
        goto err;
    }
    if (!SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_WIN32_HWND_POINTER, hwnd)) {
        error__display_fatal(SDL_LOG_CATEGORY_SYSTEM, "Failed to set SDL window properties: %s", SDL_GetError());
        goto err;
    }

    // Create window
    self->window = SDL_CreateWindowWithProperties(props);
    if (self->window == nullptr) {
        error__display_fatal(SDL_LOG_CATEGORY_SYSTEM, "Failed to create SDL window: %s", SDL_GetError());
        goto err;
    }

    SDL_DestroyProperties(props);
    props = 0;

    // Set up window mode
    if (!init_window(self->window, mode)) {
        goto err;
    }

    // Create GPU device and set up
    self->device = SDL_CreateGPUDevice(
        SDL_GPU_SHADERFORMAT_SPIRV,
        true,
        nullptr
    );
    if (self->device == nullptr) {
        error__display_fatal(SDL_LOG_CATEGORY_GPU, "Failed to acquire GPU device: %s", SDL_GetError());
        goto err;
    }
    if (!SDL_ClaimWindowForGPUDevice(self->device, self->window)) {
        error__display_fatal(SDL_LOG_CATEGORY_GPU, "Failed to claim window for GPU: %s", SDL_GetError());
        goto err;
    }

    // Init tessellator
    if (!tessellator__init(&self->tessellator, self->device)) {
        error__display_fatal(SDL_LOG_CATEGORY_APPLICATION, "Failed to init tessellator");
        goto err;
    }

    // Init surface manager
    if (!surface_manager__init(&self->surfaces)) {
        error__display_fatal(SDL_LOG_CATEGORY_APPLICATION, "Failed to init surface manager");
        goto err;
    }

    // Init blitter
    if (!blitter__init(&self->blitter, self->device, &self->tessellator)) {
        error__display_fatal(SDL_LOG_CATEGORY_APPLICATION, "Failed to init blitter");
        goto err;
    }

    // Init shaders
    SDL_GPUColorTargetDescription color_target = {
        .format = SDL_GetGPUSwapchainTextureFormat(self->device, self->window),
    };
    for (shader_id_t i = 0; i < NUM_SHADER_IDS; i++) {
        if (!shader__init_from_def(
            &self->shaders[i],
            self->device,
            &SHADER_DEFS[i],
            1,
            &color_target
        )) {
            error__display_fatal(SDL_LOG_CATEGORY_GPU, "Failed to init shader %d");
            goto err;
        }
    }

    self->initialized = true;

    return true;

err:
    SDL_DestroyProperties(props);
    renderer__cleanup(self);

    return false;
}

void renderer__cleanup(renderer_t* self) {
    if (!self->initialized) {
        return;
    }

    // Clean up shaders
    for (shader_id_t i = 0; i < NUM_SHADER_IDS; i++) {
        shader__cleanup(&self->shaders[i]);
    }

    surface_manager__cleanup(&self->surfaces);

    // Clean up SDL
    SDL_DestroyGPUDevice(self->device);
    self->device = nullptr;
    SDL_DestroyWindow(self->window);
    self->window = nullptr;
    SDL_Quit();

    self->initialized = false;
}

void renderer__clear(renderer_t* self, float r, float g, float b, float a) {
    self->clear_color[0] = r;
    self->clear_color[1] = g;
    self->clear_color[2] = b;
    self->clear_color[3] = a;
    self->should_clear = true;
}

void renderer__start_draw(renderer_t* self) {
    SDL_GPUCommandBuffer* cmd_buffer = nullptr;
    SDL_GPURenderPass* render_pass = nullptr;

    if (self->cmd_buffer != nullptr) {
        return;
    }

    // Get command buffer
    cmd_buffer = SDL_AcquireGPUCommandBuffer(self->device);
    if (cmd_buffer == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_GPU, "Could not acquire GPU command buffer: %s", SDL_GetError());
        goto err;
    }

    // Flush tessellator
    if (tessellator__needs_flush(&self->tessellator)) {
        SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(cmd_buffer);
        if (copy_pass == nullptr) {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "Could not begin GPU copy pass: %s", SDL_GetError());
            goto err;
        }

        tessellator__flush_to_gpu(&self->tessellator, cmd_buffer, copy_pass);

        SDL_EndGPUCopyPass(copy_pass);
    }

    // Get swapchain texture
    SDL_GPUTexture* swapchain_tex;
    uint32_t width, height;
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(
        cmd_buffer,
        self->window,
        &swapchain_tex,
        &width,
        &height
    )) {
        SDL_LogError(SDL_LOG_CATEGORY_GPU, "Could not acquire GPU swapchain texture: %s", SDL_GetError());
        goto err;
    }

    // Set up color target
    SDL_GPUColorTargetInfo color_target = {
        .texture = swapchain_tex,
        .store_op = SDL_GPU_STOREOP_STORE,
    };
    if (self->should_clear) {
        color_target.load_op = SDL_GPU_LOADOP_CLEAR;
        color_target.clear_color.r = self->clear_color[0];
        color_target.clear_color.g = self->clear_color[1];
        color_target.clear_color.b = self->clear_color[2];
        color_target.clear_color.a = self->clear_color[3];
    } else {
        color_target.load_op = SDL_GPU_LOADOP_LOAD;
    }

    // Start render pass
    render_pass = SDL_BeginGPURenderPass(
        cmd_buffer,
        &color_target,
        1,
        nullptr
    );
    if (render_pass == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_GPU, "Could not begin GPU render pass: %s", SDL_GetError());
        goto err;
    }

    self->cmd_buffer = cmd_buffer;
    self->render_pass = render_pass;

    return;

err:
    SDL_EndGPURenderPass(render_pass);
    SDL_SubmitGPUCommandBuffer(cmd_buffer);
}

void renderer__end_draw(renderer_t* self) {
    if (self->render_pass != nullptr) {
        SDL_EndGPURenderPass(self->render_pass);
        self->render_pass = nullptr;
    }
    if (self->cmd_buffer != nullptr) {
        SDL_SubmitGPUCommandBuffer(self->cmd_buffer);
        self->cmd_buffer = nullptr;
    }
}

void renderer__draw_scene(renderer_t* self, SceneDesc_t const* scene_desc) {
    if (scene_desc->m_DrawMode == SceneDrawMode_Normal) {
        printf("Drawing normal scene!\n");
    } else if (scene_desc->m_DrawMode == SceneDrawMode_ObjectList) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Drawing object list scene! (%u objects)\n", scene_desc->m_nObjectListSize);

        for (size_t i = 0; i < scene_desc->m_nObjectListSize; i++) {
            object__draw(scene_desc->m_pObjectList[i]);
        }
    } else {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unexpected scene draw mode! (%d)\n", scene_desc->m_DrawMode);
    }
}

void renderer__blit_to_screen(renderer_t* self, BlitRequest_t const* request) {
    blit__blit_to_screen(&self->blitter, request, self->render_pass);
}

static bool init_window(SDL_Window* window, RMode_t const* rmode) {
    SDL_DisplayMode** display_modes = nullptr;

    if (strncmp(rmode->m_InternalName, "windowed", sizeof(rmode->m_InternalName)) == 0) {
        if (!SDL_SetWindowSize(window, rmode->m_Width, rmode->m_Height)) {
            error__display_fatal(SDL_LOG_CATEGORY_VIDEO, "Failed to set window size: %s", SDL_GetError());
            goto err;
        }
        if (!SDL_SetWindowFullscreen(window, false)) {
            error__display_fatal(SDL_LOG_CATEGORY_VIDEO, "Failed to set windowed: %s", SDL_GetError());
            goto err;
        }
    } else {
        int num_display_modes = 0;
        SDL_DisplayID display_id;
        if (sscanf(rmode->m_InternalName, "sdl:%u", &display_id) != 1) {
            goto err;
        }

        bool set = false;
        display_modes = SDL_GetFullscreenDisplayModes(display_id, &num_display_modes);
        if (display_modes == nullptr || num_display_modes == 0) {
            goto err;
        }

        for (int i = 0; i < num_display_modes; i++) {
            SDL_DisplayMode* mode = display_modes[i];

            if (rmode->m_Width == mode->w && rmode->m_Height == mode->h && rmode->m_BitDepth == SDL_BITSPERPIXEL(mode->format)) {
                set = true;
                if (!SDL_SetWindowFullscreenMode(window, mode)) {
                    error__display_fatal(SDL_LOG_CATEGORY_VIDEO, "Failed to set display mode: %s", SDL_GetError());
                    goto err;
                }
                if (!SDL_SetWindowFullscreen(window, true)) {
                    error__display_fatal(SDL_LOG_CATEGORY_VIDEO, "Failed to set fullscreen: %s", SDL_GetError());
                    goto err;
                }

                break;
            }
        }

        SDL_free(display_modes);
        display_modes = nullptr;

        if (!set) {
            goto err;
        }
    }

    return true;

err:
    SDL_free(display_modes);
    return false;
}