#ifndef LMFW
#define LMFW

#define _GNU_SOURCE

#ifdef LMFW_WL
#include <xdg-shell.h>
#endif

#ifdef LMFW_XCB
#include <xcb/xcb.h>
#endif

#ifdef LMFW_XLIB
#include <X11/Xlib.h>
#endif

#include <stdint.h>
#include <stdbool.h>

#define LMFW_WINDOW_INVALID UINT32_MAX
#define LMFW_WL_VERSION 4

typedef enum lmfw_key {
    LMFW_KEY_RESERVED = 0,
    LMFW_KEY_ESC = 1,
    LMFW_KEY_1 = 2,
    LMFW_KEY_2 = 3,
    LMFW_KEY_3 = 4,
    LMFW_KEY_4 = 5,
    LMFW_KEY_5 = 6,
    LMFW_KEY_6 = 7,
    LMFW_KEY_7 = 8,
    LMFW_KEY_8 = 9,
    LMFW_KEY_9 = 10,
    LMFW_KEY_0 = 11,
    LMFW_KEY_MINUS = 12,
    LMFW_KEY_EQUAL = 13,
    LMFW_KEY_BACKSPACE = 14,
    LMFW_KEY_TAB = 15,
    LMFW_KEY_Q = 16,
    LMFW_KEY_W = 17,
    LMFW_KEY_E = 18,
    LMFW_KEY_R = 19,
    LMFW_KEY_T = 20,
    LMFW_KEY_Y = 21,
    LMFW_KEY_U = 22,
    LMFW_KEY_I = 23,
    LMFW_KEY_O = 24,
    LMFW_KEY_P = 25,
    LMFW_KEY_LEFTBRACE = 26,
    LMFW_KEY_RIGHTBRACE = 27,
    LMFW_KEY_ENTER = 28,
    LMFW_KEY_LEFTCTRL = 29,
    LMFW_KEY_A = 30,
    LMFW_KEY_S = 31,
    LMFW_KEY_D = 32,
    LMFW_KEY_F = 33,
    LMFW_KEY_G = 34,
    LMFW_KEY_H = 35,
    LMFW_KEY_J = 36,
    LMFW_KEY_K = 37,
    LMFW_KEY_L = 38,
    LMFW_KEY_SEMICOLON = 39,
    LMFW_KEY_APOSTROPHE = 40,
    LMFW_KEY_GRAVE = 41,
    LMFW_KEY_LEFTSHIFT = 42,
    LMFW_KEY_BACKSLASH = 43,
    LMFW_KEY_Z = 44,
    LMFW_KEY_X = 45,
    LMFW_KEY_C = 46,
    LMFW_KEY_V = 47,
    LMFW_KEY_B = 48,
    LMFW_KEY_N = 49,
    LMFW_KEY_M = 50,
    LMFW_KEY_COMMA = 51,
    LMFW_KEY_DOT = 52,
    LMFW_KEY_SLASH = 53,
    LMFW_KEY_RIGHTSHIFT = 54,
    LMFW_KEY_KPASTERISK = 55,
    LMFW_KEY_LEFTALT = 56,
    LMFW_KEY_SPACE = 57,
    LMFW_KEY_CAPSLOCK = 58,
    LMFW_KEY_F1 = 59,
    LMFW_KEY_F2 = 60,
    LMFW_KEY_F3 = 61,
    LMFW_KEY_F4 = 62,
    LMFW_KEY_F5 = 63,
    LMFW_KEY_F6 = 64,
    LMFW_KEY_F7 = 65,
    LMFW_KEY_F8 = 66,
    LMFW_KEY_F9 = 67,
    LMFW_KEY_F10 = 68,
    LMFW_KEY_NUMLOCK = 69,
    LMFW_KEY_SCROLLLOCK = 70,
    LMFW_KEY_KP7 = 71,
    LMFW_KEY_KP8 = 72,
    LMFW_KEY_KP9 = 73,
    LMFW_KEY_KPMINUS = 74,
    LMFW_KEY_KP4 = 75,
    LMFW_KEY_KP5 = 76,
    LMFW_KEY_KP6 = 77,
    LMFW_KEY_KPPLUS = 78,
    LMFW_KEY_KP1 = 79,
    LMFW_KEY_KP2 = 80,
    LMFW_KEY_KP3 = 81,
    LMFW_KEY_KP0 = 82,
    LMFW_KEY_KPDOT = 83,

    LMFW_KEY_F11 = 87,
    LMFW_KEY_F12 = 88,

    LMFW_KEY_KPENTER = 96,
    LMFW_KEY_RIGHTCTRL = 97,
    LMFW_KEY_KPSLASH = 98,
    LMFW_KEY_PRINTSCREEN = 99,
    LMFW_KEY_RIGHTALT = 100,

    LMFW_KEY_HOME = 102,
    LMFW_KEY_UP = 103,
    LMFW_KEY_PAGEUP = 104,
    LMFW_KEY_LEFT = 105,
    LMFW_KEY_RIGHT = 106,
    LMFW_KEY_END = 107,
    LMFW_KEY_DOWN = 108,
    LMFW_KEY_PAGEDOWN = 109,
    LMFW_KEY_INSERT = 110,
    LMFW_KEY_DELETE = 111,

    LMFW_KEY_LEFTMETA = 125,
    LMFW_KEY_RIGHTMETA = 126,
} lmfw_key;

typedef uint8_t lmfw_window_type;

#define LMFW_WINDOW_TYPE_GRAPHICS_BACKEND_NONE     ((lmfw_window_type)0b00000000)
#define LMFW_WINDOW_TYPE_GRAPHICS_BACKEND_SOFTWARE ((lmfw_window_type)0b00000001)

#define LMFW_WINDOW_TYPE_WINDOW_BACKEND_NONE       ((lmfw_window_type)0b00000000)
#define LMFW_WINDOW_TYPE_WINDOW_BACKEND_WL         ((lmfw_window_type)0b00000010)
#define LMFW_WINDOW_TYPE_WINDOW_BACKEND_XCB        ((lmfw_window_type)0b00000100)
#define LMFW_WINDOW_TYPE_WINDOW_BACKEND_XLIB       ((lmfw_window_type)0b00000110)
#define LMFW_WINDOW_TYPE_WINDOW_BACKEND_WIN32      ((lmfw_window_type)0b00001000)
#define LMFW_WINDOW_TYPE_WINDOW_BACKEND_COCOA      ((lmfw_window_type)0b00001010)

typedef uint32_t lmfw_window;
typedef int32_t lmfw_fixed_point;

#ifdef LMFW_WL
typedef struct lmfw_wl_window_data {
    struct wl_surface* wlSurface;
    struct xdg_surface* xdgSurface;
    struct xdg_toplevel* xdgToplevel;
    struct zxdg_toplevel_decoration_v1* xdgDecorations;
    struct wl_buffer* wlBuffer;

    bool decorationsEnabled;
} lmfw_wl_window_data;
#endif

#ifdef LMFW_XCB
typedef struct lmfw_xcb_window_data {
    xcb_window_t xcbWindow;
    xcb_gcontext_t xcbGraphicsContext;
} lmfw_xcb_window_data;
#endif

#ifdef LMFW_XLIB
typedef struct lmfw_xlib_window_data {
    GC xlibGraphicsContext;
    Window xlibWindow;
} lmfw_xlib_window_data;
#endif

typedef struct lmfw_window_data {
    union {
        #ifdef LMFW_WL
        lmfw_wl_window_data* wlWindowData;
        #endif
        #ifdef LMFW_XCB
        lmfw_xcb_window_data* xcbWindowData;
        #endif
        #ifdef LMFW_XLIB
        lmfw_xlib_window_data* xlibWindowData;
        #endif
    } platformWindowData;

    uint8_t* pixels;

    lmfw_fixed_point pointerX;
    lmfw_fixed_point pointerY;

    uint16_t width;
    uint16_t height;

    lmfw_window_type windowType;
    bool keyStates[139];
    bool open;
} lmfw_window_data;

#ifdef __cplusplus
extern "C" {
#endif

lmfw_window_type lmfw_get_window_type(lmfw_window window);
lmfw_window_type lmfw_get_graphics_backend(lmfw_window window);
lmfw_window_type lmfw_get_window_backend(lmfw_window window);
uint32_t lmfw_get_width(lmfw_window window);
uint32_t lmfw_get_height(lmfw_window window);
bool lmfw_window_is_open(lmfw_window window);
bool lmfw_is_key_down(lmfw_window window, lmfw_key key);
lmfw_fixed_point lmfw_pointer_get_x(lmfw_window window);
lmfw_fixed_point lmfw_pointer_get_y(lmfw_window window);
void lmfw_pointer_set_x(lmfw_window window, lmfw_fixed_point pointerX);
void lmfw_pointer_set_y(lmfw_window window, lmfw_fixed_point pointerY);
uint8_t* lmfw_get_pixels(lmfw_window window);
int32_t lmfw_fixed_point_to_int(lmfw_fixed_point fixedPoint);
int32_t lmfw_fixed_point_to_int_round(lmfw_fixed_point fixedPoint);
double lmfw_fixed_point_to_double(lmfw_fixed_point fixedPoint);

#ifdef LMFW_WL
void lmfw_wl_init();
lmfw_window lmfw_wl_create_window(uint32_t width, uint32_t height, const char* title, lmfw_window_type windowType, bool windowDecorations, bool inputPassthrough);
struct wl_display* lmfw_wl_get_display();
struct wl_surface* lmfw_wl_get_surface(lmfw_window window);
void lmfw_wl_poll_events(bool blocking);
void lmfw_wl_submit_frame(lmfw_window window);
void lmfw_wl_close_window(lmfw_window window);
void lmfw_wl_shutdown(void);
#endif

#ifdef LMFW_XCB
void lmfw_xcb_init();
lmfw_window lmfw_xcb_create_window(int16_t x, int16_t y, uint16_t width, uint16_t height, const char* title, lmfw_window_type windowType, bool inputPassthrough);
xcb_connection_t* lmfw_xcb_get_connection();
xcb_window_t lmfw_xcb_get_window(lmfw_window window);
void lmfw_xcb_poll_events(bool blocking);
void lmfw_xcb_submit_frame(lmfw_window window);
void lmfw_xcb_close_window(lmfw_window window);
void lmfw_xcb_shutdown(void);
#endif

#ifdef LMFW_XLIB
void lmfw_xlib_init();
lmfw_window lmfw_xlib_create_window(int16_t x, int16_t y, uint16_t width, uint16_t height, const char* title, lmfw_window_type windowType, bool inputPassthrough);
Display* lmfw_xlib_get_display();
Window lmfw_xlib_get_window(lmfw_window window);
void lmfw_xlib_poll_events(bool blocking);
void lmfw_xlib_submit_frame(lmfw_window window);
void lmfw_xlib_close_window(lmfw_window window);
void lmfw_xlib_shutdown(void);
#endif

#ifdef __cplusplus
}
#endif

#endif
