#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>

#include <SDL2/SDL.h>
#include <EGL/egl.h>
#include <GLES3/gl32.h>

#include "../rt64es/rt64es.h"
#include "../rt64es/src/zelda/rt64es_zelda.h"

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768
#define GAME_TITLE "Zelda Ocarina of Time"

static volatile sig_atomic_t running = 1;
static SDL_Window* g_window = NULL;
static SDL_GLContext g_gl_context = NULL;
static EGLDisplay g_egl_display = EGL_NO_DISPLAY;
static EGLSurface g_egl_surface = EGL_NO_SURFACE;
static EGLContext g_egl_context = EGL_NO_CONTEXT;

static void signal_handler(int sig) {
    (void)sig;
    running = 0;
}

static int init_egl(void) {
    g_egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (g_egl_display == EGL_NO_DISPLAY) {
        fprintf(stderr, "Failed to get EGL display\n");
        return -1;
    }
    
    if (!eglInitialize(g_egl_display, NULL, NULL)) {
        fprintf(stderr, "Failed to initialize EGL\n");
        return -1;
    }
    
    EGLint config_attribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_STENCIL_SIZE, 8,
        EGL_NONE
    };
    
    EGLConfig config;
    EGLint num_configs;
    if (!eglChooseConfig(g_egl_display, config_attribs, &config, 1, &num_configs)) {
        fprintf(stderr, "Failed to choose EGL config\n");
        return -1;
    }
    
    EGLint context_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_CONTEXT_MAJOR_VERSION, 3,
        EGL_CONTEXT_MINOR_VERSION, 2,
        EGL_NONE
    };
    
    g_egl_context = eglCreateContext(g_egl_display, config, EGL_NO_CONTEXT, context_attribs);
    if (g_egl_context == EGL_NO_CONTEXT) {
        fprintf(stderr, "Failed to create EGL context\n");
        return -1;
    }
    
    g_egl_surface = eglCreateWindowSurface(g_egl_display, config, (EGLNativeWindowType)SDL_GetNativeWindow(g_window), NULL);
    if (g_egl_surface == EGL_NO_SURFACE) {
        fprintf(stderr, "Failed to create EGL surface\n");
        return -1;
    }
    
    if (!eglMakeCurrent(g_egl_display, g_egl_surface, g_egl_surface, g_egl_context)) {
        fprintf(stderr, "Failed to make EGL context current\n");
        return -1;
    }
    
    return 0;
}

static int init_sdl(void) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0) {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        return -1;
    }
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    
    g_window = SDL_CreateWindow(GAME_TITLE, 
                               SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               SCREEN_WIDTH, SCREEN_HEIGHT,
                               SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    
    if (!g_window) {
        fprintf(stderr, "Failed to create SDL window: %s\n", SDL_GetError());
        return -1;
    }
    
    g_gl_context = SDL_GL_CreateContext(g_window);
    if (!g_gl_context) {
        fprintf(stderr, "Failed to create GL context: %s\n", SDL_GetError());
        SDL_DestroyWindow(g_window);
        return -1;
    }
    
    SDL_GL_SetSwapInterval(1);
    
    return 0;
}

static void cleanup(void) {
    if (g_egl_display != EGL_NO_DISPLAY) {
        if (g_egl_context != EGL_NO_CONTEXT) {
            eglDestroyContext(g_egl_display, g_egl_context);
        }
        if (g_egl_surface != EGL_NO_SURFACE) {
            eglDestroySurface(g_egl_display, g_egl_surface);
        }
        eglTerminate(g_egl_display);
    }
    
    if (g_gl_context) {
        SDL_GL_DeleteContext(g_gl_context);
    }
    
    if (g_window) {
        SDL_DestroyWindow(g_window);
    }
    
    SDL_Quit();
}

static int check_rom_file(const char* path) {
    struct stat st;
    if (stat(path, &st) != 0) {
        return 0;
    }
    if (st.st_size < 8 * 1024 * 1024 || st.st_size > 64 * 1024 * 1024) {
        return 0;
    }
    return 1;
}

static int process_events(void) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                running = 0;
                return 0;
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = 0;
                    return 0;
                }
                break;
        }
    }
    return 1;
}

static void render_frame(void) {
    RT64ES_Error err = rt64es_zelda_render_frame();
    if (err != RT64ES_SUCCESS) {
        fprintf(stderr, "Render frame error: %s\n", rt64es_get_error_string(err));
    }
    
    if (g_egl_display != EGL_NO_DISPLAY && g_egl_surface != EGL_NO_SURFACE) {
        eglSwapBuffers(g_egl_display, g_egl_surface);
    }
}

int main(int argc, char** argv) {
    printf("=====================================\n");
    printf("  Zelda Ocarina of Time - RT64ES\n");
    printf("=====================================\n\n");
    
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    const char* rom_path = getenv("ZELDA_ROM_PATH");
    if (!rom_path && argc > 1) {
        rom_path = argv[1];
    }
    
    if (!rom_path) {
        printf("Usage: %s <zelda_oot.n64>\n", argv[0]);
        printf("Or set ZELDA_ROM_PATH environment variable\n\n");
        printf("Note: Zelda64Recomp ROM integration requires:\n");
        printf("  1. N64 ROM file\n");
        printf("  2. Zelda64Recomp recompilation\n");
        printf("  3. ROM assets extraction\n\n");
        printf("For now, running in demo mode...\n");
        rom_path = NULL;
    } else {
        if (!check_rom_file(rom_path)) {
            fprintf(stderr, "Invalid ROM file: %s\n", rom_path);
            return 1;
        }
        printf("ROM: %s\n", rom_path);
    }
    
    printf("\nInitializing SDL...\n");
    if (init_sdl() < 0) {
        fprintf(stderr, "Failed to initialize SDL\n");
        return 1;
    }
    printf("SDL initialized\n");
    
    printf("Initializing EGL...\n");
    if (init_egl() < 0) {
        fprintf(stderr, "Failed to initialize EGL\n");
        cleanup();
        return 1;
    }
    printf("EGL initialized\n");
    
    const char* version = (const char*)glGetString(GL_VERSION);
    const char* renderer = (const char*)glGetString(GL_RENDERER);
    printf("\nOpenGL ES: %s\n", version ? version : "Unknown");
    printf("Renderer: %s\n", renderer ? renderer : "Unknown");
    
    printf("\nInitializing RT64ES...\n");
    RT64ES_WindowConfig config = {
        .window = g_window,
        .context = g_gl_context,
        .width = SCREEN_WIDTH,
        .height = SCREEN_HEIGHT,
        .fullscreen = false,
        .vsync_enabled = true
    };
    
    RT64ES_Error err = rt64es_zelda_init(&config);
    if (err != RT64ES_SUCCESS) {
        fprintf(stderr, "Failed to initialize RT64ES: %s\n", rt64es_get_error_string(err));
        cleanup();
        return 1;
    }
    printf("RT64ES initialized\n");
    
    printf("\n=====================================\n");
    printf("  Starting Game Loop\n");
    printf("  Press ESC or Ctrl+C to exit\n");
    printf("=====================================\n\n");
    
    Uint32 frame_count = 0;
    Uint32 last_fps_time = SDL_GetTicks();
    float fps = 0.0f;
    
    while (running) {
        if (!process_events()) {
            break;
        }
        
        render_frame();
        
        frame_count++;
        Uint32 current_time = SDL_GetTicks();
        if (current_time - last_fps_time >= 1000) {
            fps = frame_count * 1000.0f / (current_time - last_fps_time);
            printf("\rFPS: %.1f | Frame: %u", fps, frame_count);
            fflush(stdout);
            frame_count = 0;
            last_fps_time = current_time;
        }
        
        SDL_Delay(1);
    }
    
    printf("\n\nShutting down...\n");
    rt64es_zelda_shutdown();
    cleanup();
    printf("Shutdown complete\n");
    
    return 0;
}
