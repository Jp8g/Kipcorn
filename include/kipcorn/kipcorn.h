#ifndef KIPCORN
#define KIPCORN

#define _GNU_SOURCE 1

#include <xdg-shell.h>
#include <xdg-decoration-unstable-v1.h>
#include <xkbcommon/xkbcommon.h>
#include <wayland-egl.h>
#include <EGL/egl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdbool.h>

#define KIPCORN_WINDOW_INVALID UINT32_MAX
#define KIPCORN_WL_VERSION 4

typedef enum kip_key {
    KIPCORN_KEY_RESERVED = 0,
    KIPCORN_KEY_ESC = 1,
    KIPCORN_KEY_1 = 2,
    KIPCORN_KEY_2 = 3,
    KIPCORN_KEY_3 = 4,
    KIPCORN_KEY_4 = 5,
    KIPCORN_KEY_5 = 6,
    KIPCORN_KEY_6 = 7,
    KIPCORN_KEY_7 = 8,
    KIPCORN_KEY_8 = 9,
    KIPCORN_KEY_9 = 10,
    KIPCORN_KEY_0 = 11,
    KIPCORN_KEY_MINUS = 12,
    KIPCORN_KEY_EQUAL = 13,
    KIPCORN_KEY_BACKSPACE = 14,
    KIPCORN_KEY_TAB = 15,
    KIPCORN_KEY_Q = 16,
    KIPCORN_KEY_W = 17,
    KIPCORN_KEY_E = 18,
    KIPCORN_KEY_R = 19,
    KIPCORN_KEY_T = 20,
    KIPCORN_KEY_Y = 21,
    KIPCORN_KEY_U = 22,
    KIPCORN_KEY_I = 23,
    KIPCORN_KEY_O = 24,
    KIPCORN_KEY_P = 25,
    KIPCORN_KEY_LEFTBRACE = 26,
    KIPCORN_KEY_RIGHTBRACE = 27,
    KIPCORN_KEY_ENTER = 28,
    KIPCORN_KEY_LEFTCTRL = 29,
    KIPCORN_KEY_A = 30,
    KIPCORN_KEY_S = 31,
    KIPCORN_KEY_D = 32,
    KIPCORN_KEY_F = 33,
    KIPCORN_KEY_G = 34,
    KIPCORN_KEY_H = 35,
    KIPCORN_KEY_J = 36,
    KIPCORN_KEY_K = 37,
    KIPCORN_KEY_L = 38,
    KIPCORN_KEY_SEMICOLON = 39,
    KIPCORN_KEY_APOSTROPHE = 40,
    KIPCORN_KEY_GRAVE = 41,
    KIPCORN_KEY_LEFTSHIFT = 42,
    KIPCORN_KEY_BACKSLASH = 43,
    KIPCORN_KEY_Z = 44,
    KIPCORN_KEY_X = 45,
    KIPCORN_KEY_C = 46,
    KIPCORN_KEY_V = 47,
    KIPCORN_KEY_B = 48,
    KIPCORN_KEY_N = 49,
    KIPCORN_KEY_M = 50,
    KIPCORN_KEY_COMMA = 51,
    KIPCORN_KEY_DOT = 52,
    KIPCORN_KEY_SLASH = 53,
    KIPCORN_KEY_RIGHTSHIFT = 54,
    KIPCORN_KEY_KPASTERISK = 55,
    KIPCORN_KEY_LEFTALT = 56,
    KIPCORN_KEY_SPACE = 57,
    KIPCORN_KEY_CAPSLOCK = 58,
    KIPCORN_KEY_F1 = 59,
    KIPCORN_KEY_F2 = 60,
    KIPCORN_KEY_F3 = 61,
    KIPCORN_KEY_F4 = 62,
    KIPCORN_KEY_F5 = 63,
    KIPCORN_KEY_F6 = 64,
    KIPCORN_KEY_F7 = 65,
    KIPCORN_KEY_F8 = 66,
    KIPCORN_KEY_F9 = 67,
    KIPCORN_KEY_F10 = 68,
    KIPCORN_KEY_NUMLOCK = 69,
    KIPCORN_KEY_SCROLLLOCK = 70,
    KIPCORN_KEY_KP7 = 71,
    KIPCORN_KEY_KP8 = 72,
    KIPCORN_KEY_KP9 = 73,
    KIPCORN_KEY_KPMINUS = 74,
    KIPCORN_KEY_KP4 = 75,
    KIPCORN_KEY_KP5 = 76,
    KIPCORN_KEY_KP6 = 77,
    KIPCORN_KEY_KPPLUS = 78,
    KIPCORN_KEY_KP1 = 79,
    KIPCORN_KEY_KP2 = 80,
    KIPCORN_KEY_KP3 = 81,
    KIPCORN_KEY_KP0 = 82,
    KIPCORN_KEY_KPDOT = 83,

    KIPCORN_KEY_F11 = 87,
    KIPCORN_KEY_F12 = 88,

    KIPCORN_KEY_KPENTER = 96,
    KIPCORN_KEY_RIGHTCTRL = 97,
    KIPCORN_KEY_KPSLASH = 98,
    KIPCORN_KEY_PRINTSCREEN = 99,
    KIPCORN_KEY_RIGHTALT = 100,

    KIPCORN_KEY_HOME = 102,
    KIPCORN_KEY_UP = 103,
    KIPCORN_KEY_PAGEUP = 104,
    KIPCORN_KEY_LEFT = 105,
    KIPCORN_KEY_RIGHT = 106,
    KIPCORN_KEY_END = 107,
    KIPCORN_KEY_DOWN = 108,
    KIPCORN_KEY_PAGEDOWN = 109,
    KIPCORN_KEY_INSERT = 110,
    KIPCORN_KEY_DELETE = 111,

    KIPCORN_KEY_LEFTMETA = 125,
    KIPCORN_KEY_RIGHTMETA = 126,
} kip_key;

typedef enum kip_graphics_backend {
    KIPCORN_GRAPHICS_BACKEND_NONE,
    KIPCORN_GRAPHICS_BACKEND_SOFTWARE,
    KIPCORN_GRAPHICS_BACKEND_OPENGL,
    KIPCORN_GRAPHICS_BACKEND_VULKAN,
} kip_graphics_backend;

typedef uint32_t kip_window;
typedef wl_fixed_t kip_fixed_point;

typedef struct kip_window_data {
    struct wl_surface* waylandSurface;
    struct xdg_surface* xdgSurface;
    struct wl_callback* callback;
    struct xdg_toplevel* toplevel;
    struct zxdg_toplevel_decoration_v1* decorations;

    struct wl_buffer* buffer;
    uint8_t* pixels;

    struct wl_egl_window* eglWindow;
    EGLSurface eglSurface;
    EGLContext eglContext;

    kip_graphics_backend graphicsBackend;
    uint16_t width;
    uint16_t height;

    bool keyStates[139];

    kip_fixed_point pointerX;
    kip_fixed_point pointerY;

    bool decorationsEnabled;
    bool frameCallbackPending;
    bool frameCanRender;
    bool open;
    bool vsync;
    bool focused;
} kip_window_data;

#ifdef __cplusplus
extern "C" {
#endif

kip_window kip_create_window(uint32_t width, uint32_t height, const char* title, kip_graphics_backend graphicsBackend, bool vsync, bool windowDecorations, bool inputPassthrough, EGLContext shareContext);
void kip_set_vsync(kip_window window, bool vsync);
bool kip_get_vsync(kip_window window);
void kip_make_egl_context_current(EGLContext context);
void kip_make_egl_surface_current(kip_window window);
uint8_t* kip_get_pixels(kip_window window);
EGLContext kip_get_egl_context(kip_window window);
EGLSurface kip_get_egl_surface(kip_window window);
uint32_t kip_get_width(kip_window window);
uint32_t kip_get_height(kip_window window);
kip_fixed_point kip_pointer_get_x(kip_window window);
kip_fixed_point kip_pointer_get_y(kip_window window);
int32_t kip_fixed_point_to_int(kip_fixed_point fixedPoint);
double kip_fixed_point_to_double(kip_fixed_point fixedPoint);
void kip_poll_events(bool blocking);
bool kip_is_key_down(kip_window window, kip_key key);
bool kip_window_is_open(kip_window window);
bool kip_frame_can_render(kip_window window);
void kip_submit_frame(kip_window window);
void kip_close_window(kip_window window);
void kip_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif