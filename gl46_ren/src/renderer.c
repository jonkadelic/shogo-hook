#include "./renderer.h"

#include <stdio.h>
#include <windows.h>

#include <SDL3/SDL.h>
#include <glad/gl.h>

#include "logger.h"
#include "render/blitter.h"
#include "render/object/objects.h"
#include "render/render_api.h"
#include "shaders/shaders.h"

#define TARGET_FPS (60)
#define TARGET_FRAME_TIME_NS (TARGET_FPS > 0 ? (SDL_NS_PER_SECOND / TARGET_FPS) : 0)

static renderer_t RENDERER = { 0 };

static void __stdcall gl_debug_log(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const* message, void const* user_param);

static bool init_display(RMode_t const* rmode);

renderer_t* renderer__get_instance(void) {
    return &RENDERER;
}

bool renderer__init(RMode_t const* rmode, void* hwnd) {
    SDL_PropertiesID props = 0;

    // Skip renderer initialization if hwnd is null or a different HWND to the one we already captured
    if (hwnd == nullptr || (RENDERER.hwnd != nullptr && hwnd != RENDERER.hwnd)) {
        goto err;
    }

    // Init window if needed
    if (RENDERER.window == nullptr) {    
        props = SDL_CreateProperties();
        if (props == 0) {
            goto err;
        }
    
        if (!SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_WIN32_HWND_POINTER, hwnd)) {
            goto err;
        }
        if (!SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN, true)) {
            goto err;
        }

        SDL_SetHint(SDL_HINT_WINDOWS_ENABLE_MESSAGELOOP, "0");
    
        RENDERER.window = SDL_CreateWindowWithProperties(props);
        // RENDERER.window = SDL_CreateWindow("Debug Window", 640, 480, SDL_WINDOW_OPENGL);
        SDL_DestroyProperties(props); props = 0;
        if (RENDERER.window == nullptr) {
            LOG_FATAL("Failed to create SDL window: %s", SDL_GetError());
            goto err;
        }
    }

    if (RENDERER.gl_context == nullptr) {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG | SDL_GL_CONTEXT_ROBUST_ACCESS_FLAG);

        RENDERER.gl_context = SDL_GL_CreateContext(RENDERER.window);
        if (RENDERER.gl_context == nullptr) {
            LOG_FATAL("Failed to create OpenGL context: %s", SDL_GetError());
            goto err;
        }
    
        int gl_version = gladLoadGL((GLADloadfunc) SDL_GL_GetProcAddress);
        if (gl_version == 0) {
            LOG_FATAL("Failed to load GLAD");
            goto err;
        }

        LOG_INFO("Started OpenGL version \"%s\"", glGetString(GL_VERSION));

        glDebugMessageCallback((GLDEBUGPROC) gl_debug_log, nullptr);
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    }

    // Set up requested display mode
    if (!init_display(rmode)) {
        LOG_FATAL("Failed to init requested display mode");
        goto err;
    }

    glViewport(0, 0, rmode->m_Width, rmode->m_Height);

    // Init surfaces
    if (!rsurface_manager__init(&RENDERER.rsurfaces)) {
        LOG_FATAL("Failed to init surface manager");
        goto err;
    }

    // Init tessellator
    if (!tessellator__init(&RENDERER.tessellator)) {
        LOG_FATAL("Failed to init tessellator");
        goto err;
    }

    // Init shaders
    for (shader_id_t i = 0; i < NUM_SHADER_IDS; i++) {
        if (!shader__init_from_def(&RENDERER.shaders[i], &SHADER_DEFS[i])) {
            LOG_FATAL("Failed to init shader %s", SHADER_DEFS[i].name);
            goto err;
        }
    }

    // Init blitter
    if (!blitter__init(&RENDERER.blitter, rmode->m_Width, rmode->m_Height, &RENDERER.rsurfaces, &RENDERER.shaders[SHADER_ID__BLIT_2D], &RENDERER.tessellator)) {
        LOG_FATAL("Failed to init blitter");
        goto err;
    }

    // Init object manager
    if (!object_manager__init(&RENDERER.objects, &RENDERER.tessellator)) {
        LOG_FATAL("Failed to init object manager");
        goto err;
    }

    // Init shared texture manager
    if (!shared_texture_manager__init(&RENDERER.shared_textures)) {
        LOG_FATAL("Failed to init shared texture manager");
        goto err;
    }

    // Init backbuffer
    if (!pixel_buffer__init_backbuffer(&RENDERER.backbuffer)) {
        LOG_FATAL("Failed to init backbuffer pixel buffer");
        goto err;
    }

    // Init screen
    if (!screen__init(&RENDERER.screen, rmode->m_Width, rmode->m_Height)) {
        LOG_FATAL("Failed to init screen");
        goto err;
    }

    // Init DirectDraw interface
    if (!ddraw_iface__init(&RENDERER.ddraw.iface, &RENDERER)) {
        LOG_FATAL("Failed to init DirectDraw interface");
        goto err;
    }

    // Init DirectDraw backbuffer
    if (!ddraw_backbuffer__init(&RENDERER.ddraw.backbuffer, &RENDERER.ddraw.iface, &RENDERER.backbuffer)) {
        LOG_FATAL("Failed to init DirectDraw backbuffer");
        goto err;
    }

    return true;

err:
    SDL_DestroyProperties(props);
    renderer__cleanup();
    return false;
}

void renderer__cleanup(void) {
    renderer__reset();

    ddraw_backbuffer__cleanup(&RENDERER.ddraw.backbuffer);
    ddraw_iface__cleanup(&RENDERER.ddraw.iface);

    screen__cleanup(&RENDERER.screen);
    pixel_buffer__cleanup(&RENDERER.backbuffer);

    SDL_GL_DestroyContext(RENDERER.gl_context);
    RENDERER.gl_context = nullptr;

    SDL_DestroyWindow(RENDERER.window);
    RENDERER.window = nullptr;
}

void renderer__reset(void) {
    if (RENDERER.world != nullptr) {
        world__cleanup(RENDERER.world);
        SDL_free(RENDERER.world);
        RENDERER.world = nullptr;
    }

    shared_texture_manager__cleanup(&RENDERER.shared_textures);

    object_manager__cleanup(&RENDERER.objects);

    blitter__cleanup(&RENDERER.blitter);

    for (shader_id_t i = 0; i < NUM_SHADER_IDS; i++) {
        shader__cleanup(&RENDERER.shaders[i]);
    }

    tessellator__cleanup(&RENDERER.tessellator);
    rsurface_manager__cleanup(&RENDERER.rsurfaces);
}

bool renderer__start_3d(void) {
    if (RENDERER.in_3d) {
        return true;
    }

    RENDERER.in_3d = true;
    return true;
}

bool renderer__end_3d(void) {
    if (!RENDERER.in_3d) {
        return false;
    }

    RENDERER.in_3d = false;
    return true;
}

bool renderer__is_in_3d(void) {
    return RENDERER.in_3d;
}

bool renderer__start_2d(void) {
    if (RENDERER.in_2d) {
        return true;
    }

    RENDERER.in_2d = true;
    return true;
}

bool renderer__end_2d(void) {
    if (!RENDERER.in_2d) {
        return false;
    }

    RENDERER.in_2d = false;
    return true;
}

bool renderer__is_in_2d(void) {
    return RENDERER.in_2d;
}

void renderer__swap_buffers(void) {
    if (RENDERER.window == nullptr || RENDERER.gl_context == nullptr) {
        return;
    }

    if (TARGET_FRAME_TIME_NS > 0) {
        uint64_t now = SDL_GetTicksNS();
        uint64_t elapsed = now - RENDERER.last_swap;
        if (RENDERER.last_swap != 0 && elapsed < TARGET_FRAME_TIME_NS) {
            SDL_DelayNS(TARGET_FRAME_TIME_NS - elapsed);
        }
    }

    SDL_GL_SwapWindow(RENDERER.window);

    RENDERER.last_swap = SDL_GetTicksNS();
}

void renderer__draw_object(DObject_t const* object) {
    object_manager__draw(&RENDERER.objects, object);
}

void renderer__set_camera(DVector_t pos, DRotation_t rotation, float fov_y, float aspect) {
    RENDERER.camera.pos = pos;
    RENDERER.camera.rotation = rotation;
    RENDERER.camera.fov_y = fov_y;
    RENDERER.camera.aspect = aspect;
}

HMM_Mat4 renderer__get_view_projection_matrix(void) {
    HMM_Mat4 camera_matrix = HMM_MulM4(
        HMM_Translate(HMM_V3(RENDERER.camera.pos.x, RENDERER.camera.pos.y, RENDERER.camera.pos.z)),
        HMM_QToM4(HMM_Q(
            RENDERER.camera.rotation.m_Vec.x, RENDERER.camera.rotation.m_Vec.y,
            RENDERER.camera.rotation.m_Vec.z, RENDERER.camera.rotation.m_Spin
        ))
    );
    // Camera world matrix inverts to the view matrix since it's a rigid transform
    HMM_Mat4 view_matrix = HMM_InvGeneralM4(camera_matrix);
    HMM_Mat4 projection_matrix = HMM_Perspective_LH_NO(RENDERER.camera.fov_y, RENDERER.camera.aspect, 1.0f, 50000.0f);

    return HMM_MulM4(projection_matrix, view_matrix);
}

rsurface_manager_t* renderer__get_rsurfaces(void) {
    return &RENDERER.rsurfaces;
}

tessellator_t* renderer__get_tessellator(void) {
    return &RENDERER.tessellator;
}

shader_t const* renderer__get_shaders(void) {
    return RENDERER.shaders;
}

blitter_t* renderer__get_blitter(void) {
    return &RENDERER.blitter;
}

shared_texture_manager_t* renderer__get_shared_textures(void) {
    return &RENDERER.shared_textures;
}

static void gl_debug_log(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const* message, void const* user_param) {
    char const* source_str;
    switch (source) {
        case GL_DEBUG_SOURCE_API: {
            source_str = "API";
        } break;
        case GL_DEBUG_SOURCE_APPLICATION: {
            source_str = "Application";
        } break;
        case GL_DEBUG_SOURCE_OTHER: {
            source_str = "Other";
        } break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: {
            source_str = "Shader Compiler";
        } break;
        case GL_DEBUG_SOURCE_THIRD_PARTY: {
            source_str = "Third-Party";
        } break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: {
            source_str = "Window System";
        } break;
        default: {
            SDL_assert(false);
        }
    }

    char const* type_str;
    switch (type) {
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: {
            type_str = "Deprecated Behavior";
        } break;
        case GL_DEBUG_TYPE_ERROR: {
            type_str = "Error";
        } break;
        case GL_DEBUG_TYPE_MARKER: {
            type_str = "Marker";
        } break;
        case GL_DEBUG_TYPE_OTHER: {
            type_str = "Other";
        } break;
        case GL_DEBUG_TYPE_PERFORMANCE: {
            type_str = "Performance";
        } break;
        case GL_DEBUG_TYPE_POP_GROUP: {
            type_str = "Pop Group";
        } break;
        case GL_DEBUG_TYPE_PORTABILITY: {
            type_str = "Portability";
        } break;
        case GL_DEBUG_TYPE_PUSH_GROUP: {
            type_str = "Push Group";
        } break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: {
            type_str = "Undefined Behavior";
        } break;
    }

    log_level_t log_level;
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH: {
            log_level = LOG_LEVEL__ERROR;
        } break;
        case GL_DEBUG_SEVERITY_MEDIUM: {
            log_level = LOG_LEVEL__WARNING;
        } break;
        case GL_DEBUG_SEVERITY_LOW: {
            log_level = LOG_LEVEL__INFO;
        } break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: {
            log_level = LOG_LEVEL__DEBUG;
        } break;
        default: {
            SDL_assert(false);
        }
    }

    logger__log(log_level, "(OpenGL %s/%s) %s", source_str, type_str, message);
}

static bool init_display(RMode_t const* rmode) {
    SDL_DisplayMode** display_modes = nullptr;

    if (strncmp(rmode->m_InternalName, "windowed", sizeof(rmode->m_InternalName)) == 0) {
        if (!SDL_SetWindowSize(RENDERER.window, rmode->m_Width, rmode->m_Height)) {
            goto err;
        }
        if (!SDL_SetWindowFullscreen(RENDERER.window, false)) {
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
        for (int i = 0; i < num_display_modes; i++) {
            SDL_DisplayMode* mode = display_modes[i];

            if (rmode->m_Width == mode->w && rmode->m_Height == mode->h && rmode->m_BitDepth == SDL_BITSPERPIXEL(mode->format)) {
                set = true;
                if (!SDL_SetWindowFullscreenMode(RENDERER.window, mode)) {
                    goto err;
                }
                if (!SDL_SetWindowFullscreen(RENDERER.window, true)) {
                    goto err;
                }
            }
        }

        if (!set) {
            goto err;
        }
    }

    SDL_free(display_modes);

    return true;

err:
    SDL_free(display_modes);
    return false;
}