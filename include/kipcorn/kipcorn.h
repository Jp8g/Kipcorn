#ifndef KIPCORN
#define KIPCORN

#define _GNU_SOURCE

#ifdef KIPCORN_WL
#include <xdg-shell.h>
#endif

#ifdef KIPCORN_XCB
#include <xcb/xcb.h>
#endif

#ifdef KIPCORN_XLIB
#include <X11/Xlib.h>
#endif

#include <stdint.h>
#include <stdbool.h>

#define KIP_WINDOW_INVALID UINT32_MAX
#define KIP_WL_VERSION 4

typedef enum kip_key {
    KIP_KEY_RESERVED = 0,
    KIP_KEY_ESC = 1,
    KIP_KEY_1 = 2,
    KIP_KEY_2 = 3,
    KIP_KEY_3 = 4,
    KIP_KEY_4 = 5,
    KIP_KEY_5 = 6,
    KIP_KEY_6 = 7,
    KIP_KEY_7 = 8,
    KIP_KEY_8 = 9,
    KIP_KEY_9 = 10,
    KIP_KEY_0 = 11,
    KIP_KEY_MINUS = 12,
    KIP_KEY_EQUAL = 13,
    KIP_KEY_BACKSPACE = 14,
    KIP_KEY_TAB = 15,
    KIP_KEY_Q = 16,
    KIP_KEY_W = 17,
    KIP_KEY_E = 18,
    KIP_KEY_R = 19,
    KIP_KEY_T = 20,
    KIP_KEY_Y = 21,
    KIP_KEY_U = 22,
    KIP_KEY_I = 23,
    KIP_KEY_O = 24,
    KIP_KEY_P = 25,
    KIP_KEY_LEFTBRACE = 26,
    KIP_KEY_RIGHTBRACE = 27,
    KIP_KEY_ENTER = 28,
    KIP_KEY_LEFTCTRL = 29,
    KIP_KEY_A = 30,
    KIP_KEY_S = 31,
    KIP_KEY_D = 32,
    KIP_KEY_F = 33,
    KIP_KEY_G = 34,
    KIP_KEY_H = 35,
    KIP_KEY_J = 36,
    KIP_KEY_K = 37,
    KIP_KEY_L = 38,
    KIP_KEY_SEMICOLON = 39,
    KIP_KEY_APOSTROPHE = 40,
    KIP_KEY_GRAVE = 41,
    KIP_KEY_LEFTSHIFT = 42,
    KIP_KEY_BACKSLASH = 43,
    KIP_KEY_Z = 44,
    KIP_KEY_X = 45,
    KIP_KEY_C = 46,
    KIP_KEY_V = 47,
    KIP_KEY_B = 48,
    KIP_KEY_N = 49,
    KIP_KEY_M = 50,
    KIP_KEY_COMMA = 51,
    KIP_KEY_DOT = 52,
    KIP_KEY_SLASH = 53,
    KIP_KEY_RIGHTSHIFT = 54,
    KIP_KEY_KPASTERISK = 55,
    KIP_KEY_LEFTALT = 56,
    KIP_KEY_SPACE = 57,
    KIP_KEY_CAPSLOCK = 58,
    KIP_KEY_F1 = 59,
    KIP_KEY_F2 = 60,
    KIP_KEY_F3 = 61,
    KIP_KEY_F4 = 62,
    KIP_KEY_F5 = 63,
    KIP_KEY_F6 = 64,
    KIP_KEY_F7 = 65,
    KIP_KEY_F8 = 66,
    KIP_KEY_F9 = 67,
    KIP_KEY_F10 = 68,
    KIP_KEY_NUMLOCK = 69,
    KIP_KEY_SCROLLLOCK = 70,
    KIP_KEY_KP7 = 71,
    KIP_KEY_KP8 = 72,
    KIP_KEY_KP9 = 73,
    KIP_KEY_KPMINUS = 74,
    KIP_KEY_KP4 = 75,
    KIP_KEY_KP5 = 76,
    KIP_KEY_KP6 = 77,
    KIP_KEY_KPPLUS = 78,
    KIP_KEY_KP1 = 79,
    KIP_KEY_KP2 = 80,
    KIP_KEY_KP3 = 81,
    KIP_KEY_KP0 = 82,
    KIP_KEY_KPDOT = 83,

    KIP_KEY_F11 = 87,
    KIP_KEY_F12 = 88,

    KIP_KEY_KPENTER = 96,
    KIP_KEY_RIGHTCTRL = 97,
    KIP_KEY_KPSLASH = 98,
    KIP_KEY_PRINTSCREEN = 99,
    KIP_KEY_RIGHTALT = 100,

    KIP_KEY_HOME = 102,
    KIP_KEY_UP = 103,
    KIP_KEY_PAGEUP = 104,
    KIP_KEY_LEFT = 105,
    KIP_KEY_RIGHT = 106,
    KIP_KEY_END = 107,
    KIP_KEY_DOWN = 108,
    KIP_KEY_PAGEDOWN = 109,
    KIP_KEY_INSERT = 110,
    KIP_KEY_DELETE = 111,

    KIP_KEY_LEFTMETA = 125,
    KIP_KEY_RIGHTMETA = 126,
} kip_key;

typedef uint8_t kip_window_type;

#define KIP_WINDOW_TYPE_GRAPHICS_BACKEND_NONE     ((kip_window_type)0b00000000)
#define KIP_WINDOW_TYPE_GRAPHICS_BACKEND_SOFTWARE ((kip_window_type)0b00000001)

#define KIP_WINDOW_TYPE_WINDOW_BACKEND_NONE       ((kip_window_type)0b00000000)
#define KIP_WINDOW_TYPE_WINDOW_BACKEND_WL         ((kip_window_type)0b00000010)
#define KIP_WINDOW_TYPE_WINDOW_BACKEND_XCB        ((kip_window_type)0b00000100)
#define KIP_WINDOW_TYPE_WINDOW_BACKEND_XLIB       ((kip_window_type)0b00000110)
#define KIP_WINDOW_TYPE_WINDOW_BACKEND_WIN32      ((kip_window_type)0b00001000)
#define KIP_WINDOW_TYPE_WINDOW_BACKEND_COCOA      ((kip_window_type)0b00001010)

typedef uint32_t kip_window;
typedef uint32_t kip_fixed_point;

#ifdef KIPCORN_WL
typedef struct kip_wl_window_data {
    struct wl_surface* wlSurface;
    struct xdg_surface* xdgSurface;
    struct xdg_toplevel* xdgToplevel;
    struct zxdg_toplevel_decoration_v1* xdgDecorations;
    struct wl_buffer* wlBuffer;

    bool decorationsEnabled;
} kip_wl_window_data;
#endif

#ifdef KIPCORN_XCB
typedef struct kip_xcb_window_data {
    xcb_window_t xcbWindow;
    xcb_gcontext_t xcbGraphicsContext;
} kip_xcb_window_data;
#endif

#ifdef KIPCORN_XLIB
typedef struct kip_xlib_window_data {
    GC xlibGraphicsContext;
    Window xlibWindow;
} kip_xlib_window_data;
#endif

typedef struct kip_window_data {
    union {
        #ifdef KIPCORN_WL
        kip_wl_window_data* wlWindowData;
        #endif
        #ifdef KIPCORN_XCB
        kip_xcb_window_data* xcbWindowData;
        #endif
        #ifdef KIPCORN_XLIB
        kip_xlib_window_data* xlibWindowData;
        #endif
    } platformWindowData;

    uint8_t* pixels;

    kip_fixed_point pointerX;
    kip_fixed_point pointerY;

    uint16_t width;
    uint16_t height;

    kip_window_type windowType;
    bool keyStates[139];
    bool open;
} kip_window_data;

#ifdef __cplusplus
extern "C" {
#endif

kip_window_type kip_get_window_type(kip_window window);
kip_window_type kip_get_graphics_backend(kip_window window);
kip_window_type kip_get_window_backend(kip_window window);
uint32_t kip_get_width(kip_window window);
uint32_t kip_get_height(kip_window window);
bool kip_window_is_open(kip_window window);
bool kip_is_key_down(kip_window window, kip_key key);
kip_fixed_point kip_pointer_get_x(kip_window window);
kip_fixed_point kip_pointer_get_y(kip_window window);
uint8_t* kip_get_pixels(kip_window window);
int32_t kip_fixed_point_to_int(kip_fixed_point fixedPoint);
int32_t kip_fixed_point_to_int_round(kip_fixed_point fixedPoint);
double kip_fixed_point_to_double(kip_fixed_point fixedPoint);

#ifdef KIPCORN_WL
void kip_wl_init();
kip_window kip_wl_create_window(uint32_t width, uint32_t height, const char* title, kip_window_type windowType, bool windowDecorations, bool inputPassthrough);
struct wl_display* kip_wl_get_display();
struct wl_surface* kip_wl_get_surface(kip_window window);
void kip_wl_poll_events(bool blocking);
void kip_wl_submit_frame(kip_window window);
void kip_wl_close_window(kip_window window);
void kip_wl_shutdown(void);
#endif

#ifdef KIPCORN_XCB
void kip_xcb_init();
kip_window kip_xcb_create_window(int16_t x, int16_t y, uint16_t width, uint16_t height, const char* title, kip_window_type windowType, bool inputPassthrough);
xcb_connection_t* kip_xcb_get_connection();
xcb_window_t kip_xcb_get_window(kip_window window);
void kip_xcb_poll_events(bool blocking);
void kip_xcb_submit_frame(kip_window window);
void kip_xcb_close_window(kip_window window);
void kip_xcb_shutdown(void);
#endif

#ifdef KIPCORN_XLIB
void kip_xlib_init();
kip_window kip_xlib_create_window(int16_t x, int16_t y, uint16_t width, uint16_t height, const char* title, kip_window_type windowType, bool inputPassthrough);
Display* kip_xlib_get_display();
Window kip_xlib_get_window(kip_window window);
void kip_xlib_poll_events(bool blocking);
void kip_xlib_submit_frame(kip_window window);
void kip_xlib_close_window(kip_window window);
void kip_xlib_shutdown(void);
#endif

#ifdef __cplusplus
}
#endif

#endif