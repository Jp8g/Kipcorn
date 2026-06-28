#include <lmfw/lmfw.h>

lmfw_window_data* lmfwWindows;
uint32_t lmfwWindowCount = 0;
uint32_t lmfwWindowCapacity = 0;

lmfw_window keyboardFocusedLmfwWindow = LMFW_WINDOW_INVALID;
lmfw_window pointerFocusedLmfwWindow = LMFW_WINDOW_INVALID;

#ifdef LMFW_WL
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-util.h>
#include <xkbcommon/xkbcommon.h>
#include <wayland/xdg-decoration-unstable-v1.h>
#include <xkbcommon/xkbcommon.h>
#include <sys/mman.h>
#include <sys/poll.h>
#include <stdlib.h>
#include <string.h>
#endif

#ifdef LMFW_XCB
#include <xcb/xcb.h>
#include <stdlib.h>
#include <string.h>
#endif

#ifdef LMFW_XLIB
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <stdlib.h>
#include <string.h>
#endif

#include <stdint.h>
#include <unistd.h>

inline lmfw_window_type lmfw_get_window_type(lmfw_window window) {
    return lmfwWindows[window].windowType;
}

inline lmfw_window_type lmfw_get_graphics_backend(lmfw_window window) {
    return lmfwWindows[window].windowType & 0b00000001;
}

inline lmfw_window_type lmfw_get_window_backend(lmfw_window window) {
    return lmfwWindows[window].windowType & 0b00001110;
}

inline uint32_t lmfw_get_width(lmfw_window window) {
    return lmfwWindows[window].width;
}

inline uint32_t lmfw_get_height(lmfw_window window) {
    return lmfwWindows[window].height;
}

inline lmfw_fixed_point lmfw_pointer_get_x(lmfw_window window) {
    return lmfwWindows[window].pointerX;
}

inline lmfw_fixed_point lmfw_pointer_get_y(lmfw_window window) {
    return lmfwWindows[window].pointerY;
}

inline void lmfw_pointer_set_x(lmfw_window window, lmfw_fixed_point pointerX) {
    lmfwWindows[window].pointerX = pointerX;
}

inline void lmfw_pointer_set_y(lmfw_window window, lmfw_fixed_point pointerY) {
    lmfwWindows[window].pointerY = pointerY;
}

inline uint8_t* lmfw_get_pixels(lmfw_window window) {
    return lmfwWindows[window].pixels;
}

inline int32_t lmfw_fixed_point_to_int(lmfw_fixed_point fixedPoint) {
    return fixedPoint >> 8;
}

inline int32_t lmfw_fixed_point_to_int_round(lmfw_fixed_point fixedPoint) {
    return (fixedPoint + 0b10000000) >> 8;
}

inline double lmfw_fixed_point_to_double(lmfw_fixed_point fixedPoint) {
    return (double)fixedPoint / 256.0;
}

inline bool lmfw_is_key_down(lmfw_window window, lmfw_key key) {
    return key < 139 && lmfwWindows[window].keyStates[key];
}

inline bool lmfw_window_is_open(lmfw_window window) {
    return lmfwWindows[window].open;
}

#ifdef LMFW_WL
void lmfw_wl_configure_xdg_surface(void* data, struct xdg_surface* surface, uint32_t serial);
void lmfw_wl_toplevel_configuration(void* data, struct xdg_toplevel* toplevel, int32_t width, int32_t height, struct wl_array* states);
void lmfw_wl_toplevel_close(void* data, struct xdg_toplevel* toplevel);
void lmfw_wl_toplevel_configure_bounds(void* data, struct xdg_toplevel* toplevel, int32_t width, int32_t height);
void lmfw_wl_toplevel_wm_capabilities(void* data, struct xdg_toplevel* toplevel, struct wl_array* states);
void lmfw_wl_seat_capabilities(void* data, struct wl_seat* seat, uint32_t capabilities);
void lmfw_wl_seat_name(void* data, struct wl_seat* seat, const char* name);
void lmfw_wl_keyboard_keymap(void* data, struct wl_keyboard* wl_keyboard, uint32_t format, int32_t fd, uint32_t size);
void lmfw_wl_keyboard_enter(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, struct wl_surface* surface, struct wl_array* keys);
void lmfw_wl_keyboard_leave(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, struct wl_surface* surface);
void lmfw_wl_keyboard_key(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
void lmfw_wl_keyboard_modifiers(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group);
void lmfw_wl_keyboard_repeat_info(void* data, struct wl_keyboard* wl_keyboard, int32_t rate, int32_t delay);
void lmfw_wl_pointer_enter(void *data, struct wl_pointer* wl_pointer, uint32_t serial, struct wl_surface* surface, wl_fixed_t surface_x, wl_fixed_t surface_y);
void lmfw_wl_pointer_leave(void *data, struct wl_pointer* wl_pointer, uint32_t serial, struct wl_surface* surface);
void lmfw_wl_pointer_motion(void *data, struct wl_pointer* wl_pointer, uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y);
void lmfw_wl_pointer_button(void *data, struct wl_pointer* wl_pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state);
void lmfw_wl_xdg_ping(void* data, struct xdg_wm_base* shell, uint32_t serial);
void lmfw_wl_registry_global(void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version);
void lmfw_wl_registry_global_remove(void* data, struct wl_registry* registry, uint32_t name);

struct xdg_surface_listener xdgSurfaceListener = {lmfw_wl_configure_xdg_surface};
struct xdg_toplevel_listener xdgToplevelListener = {lmfw_wl_toplevel_configuration, lmfw_wl_toplevel_close, lmfw_wl_toplevel_configure_bounds, lmfw_wl_toplevel_wm_capabilities};
struct xdg_wm_base_listener shListener = {lmfw_wl_xdg_ping};
struct wl_registry_listener registryListener = {lmfw_wl_registry_global, lmfw_wl_registry_global_remove};

struct wl_compositor* wlCompositor;
struct wl_display* wlDisplay;
struct wl_registry* wlRegistry;
struct xdg_wm_base* xdgShell;
struct zxdg_decoration_manager_v1* xdgDecorationManager;

struct wl_shm* wlSharedMemory;

struct wl_seat_listener wlSeatListener = {lmfw_wl_seat_capabilities, lmfw_wl_seat_name};

struct wl_keyboard_listener wlKeyboardListener = {lmfw_wl_keyboard_keymap, lmfw_wl_keyboard_enter, lmfw_wl_keyboard_leave, lmfw_wl_keyboard_key, lmfw_wl_keyboard_modifiers, lmfw_wl_keyboard_repeat_info};

struct wl_pointer_listener wlPointerListener = {lmfw_wl_pointer_enter, lmfw_wl_pointer_leave, lmfw_wl_pointer_motion, lmfw_wl_pointer_button};

struct xkb_context* xkbContext;
struct wl_seat* wlSeat;

struct wl_keyboard* wlKeyboard;
struct xkb_keymap* xkbKeymap;
struct xkb_state* xkbState;

struct wl_pointer* wlPointer;

bool wlInit = false;

void lmfw_wl_init() {
    wlInit = true;
    wlDisplay = wl_display_connect(NULL);
    wlRegistry = wl_display_get_registry(wlDisplay);
    wl_registry_add_listener(wlRegistry, &registryListener, NULL);
    wl_display_roundtrip(wlDisplay);
}

lmfw_window lmfw_wl_create_window(uint32_t width, uint32_t height, const char* title, lmfw_window_type windowType, bool windowDecorations, bool inputPassthrough) {
    if (!wlInit) {
        return LMFW_WINDOW_INVALID;
    }

    lmfwWindowCount++;

    uint32_t oldLmfwWindowCapacity = lmfwWindowCapacity;
    if (lmfwWindowCount >= lmfwWindowCapacity) {
        lmfwWindowCapacity = lmfwWindowCapacity ? lmfwWindowCapacity * 2 : 1;
        lmfwWindows = realloc(lmfwWindows, lmfwWindowCapacity * sizeof(lmfw_window_data));

        memset(&lmfwWindows[oldLmfwWindowCapacity], 0, (lmfwWindowCapacity - oldLmfwWindowCapacity) * sizeof(lmfw_window_data));
    }

    lmfw_window window = lmfwWindowCount - 1;
    lmfw_window_data* windowData = &lmfwWindows[window];

    windowData->width = width;
    windowData->height = height;
    windowData->windowType = (windowType & 0b00000001) | LMFW_WINDOW_TYPE_WINDOW_BACKEND_WL;
    windowData->platformWindowData.wlWindowData = malloc(sizeof(lmfw_wl_window_data));
    windowData->platformWindowData.wlWindowData->decorationsEnabled = windowDecorations;
    windowData->open = false;

    windowData->platformWindowData.wlWindowData->wlSurface = wl_compositor_create_surface(wlCompositor);

    windowData->platformWindowData.wlWindowData->xdgSurface = xdg_wm_base_get_xdg_surface(xdgShell, windowData->platformWindowData.wlWindowData->wlSurface);

    xdg_surface_add_listener(windowData->platformWindowData.wlWindowData->xdgSurface, &xdgSurfaceListener, (void*)(uintptr_t)window);
    windowData->platformWindowData.wlWindowData->xdgToplevel = xdg_surface_get_toplevel(windowData->platformWindowData.wlWindowData->xdgSurface);
    xdg_toplevel_add_listener(windowData->platformWindowData.wlWindowData->xdgToplevel, &xdgToplevelListener, (void*)(uintptr_t)window);
    xdg_toplevel_set_title(windowData->platformWindowData.wlWindowData->xdgToplevel, title);
    wl_surface_commit(windowData->platformWindowData.wlWindowData->wlSurface);

    if (windowData->platformWindowData.wlWindowData->decorationsEnabled) {
        windowData->platformWindowData.wlWindowData->xdgDecorations = zxdg_decoration_manager_v1_get_toplevel_decoration(xdgDecorationManager, windowData->platformWindowData.wlWindowData->xdgToplevel);
        zxdg_toplevel_decoration_v1_set_mode(windowData->platformWindowData.wlWindowData->xdgDecorations, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
    }

    if (inputPassthrough) {
        struct wl_region* region = wl_compositor_create_region(wlCompositor);
        wl_surface_set_input_region(windowData->platformWindowData.wlWindowData->wlSurface, region);
        wl_region_destroy(region);
    }

    windowData->open = true;

    return window;
}

struct wl_display* lmfw_wl_get_display() {
    return wlDisplay;
}

struct wl_surface* lmfw_wl_get_surface(lmfw_window window) {
    return lmfwWindows[window].platformWindowData.wlWindowData->wlSurface;
}

void lmfw_wl_poll_events(bool blocking) {
    if (blocking) {
        wl_display_dispatch(wlDisplay);
        return;
    }

    wl_display_dispatch_pending(wlDisplay);

    if (wl_display_prepare_read(wlDisplay) == 0) {
        struct pollfd pfd = {
            .fd = wl_display_get_fd(wlDisplay),
            .events = POLLIN
        };

        wl_display_flush(wlDisplay);

        if (poll(&pfd, 1, 0) > 0 && (pfd.revents & POLLIN)) {
            wl_display_read_events(wlDisplay);
        } else {
            wl_display_cancel_read(wlDisplay);
        }

        wl_display_dispatch_pending(wlDisplay);
    }
}

void lmfw_wl_submit_frame(lmfw_window window) {
    lmfw_window_data* windowData = &lmfwWindows[window];

    switch (lmfw_get_graphics_backend(window)) {
        case LMFW_WINDOW_TYPE_GRAPHICS_BACKEND_NONE: {
            break;
        }

        case LMFW_WINDOW_TYPE_GRAPHICS_BACKEND_SOFTWARE: {
            if (!windowData->pixels) return;

            wl_surface_attach(windowData->platformWindowData.wlWindowData->wlSurface, windowData->platformWindowData.wlWindowData->wlBuffer, 0, 0);
            wl_surface_damage(windowData->platformWindowData.wlWindowData->wlSurface, 0, 0, windowData->width, windowData->height);
            wl_surface_commit(windowData->platformWindowData.wlWindowData->wlSurface);
            break;
        }

        default: {
            break;
        }
    }
}

void lmfw_wl_close_window(lmfw_window window) {
    lmfw_window_data* windowData = &lmfwWindows[window];

    switch (lmfw_get_graphics_backend(window)) {
        case LMFW_WINDOW_TYPE_GRAPHICS_BACKEND_NONE: {
            break;
        }

        case LMFW_WINDOW_TYPE_GRAPHICS_BACKEND_SOFTWARE: {
            if (windowData->platformWindowData.wlWindowData->wlBuffer) wl_buffer_destroy(windowData->platformWindowData.wlWindowData->wlBuffer);
            break;
        }

        default: {
            break;
        }
    }

    if (windowData->platformWindowData.wlWindowData->decorationsEnabled) {
        zxdg_toplevel_decoration_v1_destroy(windowData->platformWindowData.wlWindowData->xdgDecorations);
        windowData->platformWindowData.wlWindowData->xdgDecorations = NULL;
    }

    xdg_toplevel_destroy(windowData->platformWindowData.wlWindowData->xdgToplevel);
    xdg_surface_destroy(windowData->platformWindowData.wlWindowData->xdgSurface);

    wl_surface_destroy(windowData->platformWindowData.wlWindowData->wlSurface);

    free(windowData->platformWindowData.wlWindowData);
    memset(windowData, 0, sizeof(lmfw_window_data));
}

void lmfw_wl_shutdown(void) {
    wlInit = false;


    for (uint32_t i = 0; i < lmfwWindowCount; i++) {

        if (lmfwWindows[i].platformWindowData.wlWindowData == NULL || lmfwWindows[i].platformWindowData.wlWindowData->wlSurface == NULL) continue;

        lmfw_wl_close_window(i);
    }

    zxdg_decoration_manager_v1_destroy(xdgDecorationManager);

    wl_keyboard_release(wlKeyboard);
    wl_seat_release(wlSeat);

    xkb_state_unref(xkbState);
    xkb_keymap_unref(xkbKeymap);
    xkb_context_unref(xkbContext);

    wl_registry_destroy(wlRegistry);
    wl_display_disconnect(wlDisplay);
    free(lmfwWindows);
}

void lmfw_wl_resize(lmfw_window window, uint32_t width, uint32_t height) {
    lmfw_window_data* windowData = &lmfwWindows[window];

    switch (lmfw_get_graphics_backend(window)) {
        case LMFW_WINDOW_TYPE_GRAPHICS_BACKEND_SOFTWARE: {
            if (windowData->pixels) munmap(windowData->pixels, windowData->width * windowData->height * 4);

            windowData->width = width;
            windowData->height = height;

            int32_t fileDescriptor = memfd_create("", MFD_CLOEXEC);
            ftruncate(fileDescriptor, width * height * 4);
            
            windowData->pixels = mmap(NULL, width*  height*  4, PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, 0);

            struct wl_shm_pool* pool = wl_shm_create_pool(wlSharedMemory, fileDescriptor, width * height * 4);
            windowData->platformWindowData.wlWindowData->wlBuffer = wl_shm_pool_create_buffer(pool, 0, width, height, width * 4, WL_SHM_FORMAT_ARGB8888);
            wl_shm_pool_destroy(pool);
            close(fileDescriptor);
            break;
        }

        default: {
            windowData->width = width;
            windowData->height = height;
            break;
        }
    }
}

void lmfw_wl_configure_xdg_surface(void* data, struct xdg_surface* surface, uint32_t serial) {
    lmfw_window_data* windowData = &lmfwWindows[(lmfw_window)(uintptr_t)data];
    if (!windowData) return;

    xdg_surface_ack_configure(surface, serial);

    if ((lmfw_get_graphics_backend((lmfw_window)(uintptr_t)data)) == LMFW_WINDOW_TYPE_GRAPHICS_BACKEND_SOFTWARE && !windowData->pixels) {
        lmfw_wl_resize((lmfw_window)(uintptr_t)data, windowData->width, windowData->height);
        lmfw_wl_submit_frame((lmfw_window)(uintptr_t)data);
    }
}

void lmfw_wl_toplevel_configuration(void* data, struct xdg_toplevel* toplevel, int32_t width, int32_t height, struct wl_array* states) {
    if (!width && !height) {
        return;
    }

    lmfw_window_data* windowData = &lmfwWindows[(lmfw_window)(uintptr_t)data];
    if (!windowData) return;

    if (windowData->width != width || windowData->height != height) {
        lmfw_wl_resize((lmfw_window)(uintptr_t)data, width, height);
    }
}

void lmfw_wl_toplevel_close(void* data, struct xdg_toplevel* toplevel) {
    lmfw_window_data* windowData = &lmfwWindows[(lmfw_window)(uintptr_t)data];

    windowData->open = false;
}

void lmfw_wl_toplevel_configure_bounds(void* data, struct xdg_toplevel* toplevel, int32_t width, int32_t height) {

}

void lmfw_wl_toplevel_wm_capabilities(void* data, struct xdg_toplevel* toplevel, struct wl_array* states) {

}

void lmfw_wl_seat_capabilities(void* data, struct wl_seat* seat, uint32_t capabilities) {
    xkbContext = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

    wlKeyboard = wl_seat_get_keyboard(seat);
    wl_keyboard_add_listener(wlKeyboard, &wlKeyboardListener, NULL);

    wlPointer = wl_seat_get_pointer(seat);
    wl_pointer_add_listener(wlPointer, &wlPointerListener, NULL);
}

void lmfw_wl_seat_name(void* data, struct wl_seat* seat, const char* name) {

}

void lmfw_wl_keyboard_keymap(void* data, struct wl_keyboard* wl_keyboard, uint32_t format, int32_t fd, uint32_t size) {
    char* keymapString = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);

    xkbKeymap = xkb_keymap_new_from_string(xkbContext, keymapString, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
    xkbState = xkb_state_new(xkbKeymap);
    munmap(keymapString, size);
    close(fd);
}

void lmfw_wl_keyboard_enter(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, struct wl_surface* surface, struct wl_array* keys) {
    for (uint32_t i = 0; i < lmfwWindowCount; i++) {
        if (lmfwWindows[i].platformWindowData.wlWindowData->wlSurface == surface) {
            keyboardFocusedLmfwWindow = i;
        }
    }
}

void lmfw_wl_keyboard_leave(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, struct wl_surface* surface) {
    for (uint32_t i = 0; i < lmfwWindowCount; i++) {
        if (lmfwWindows[i].platformWindowData.wlWindowData->wlSurface == surface && keyboardFocusedLmfwWindow == i) {
            keyboardFocusedLmfwWindow = LMFW_WINDOW_INVALID;
        }
    }
}

void lmfw_wl_keyboard_key(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
    if (!lmfwWindows) return;

    xkb_state_update_key(xkbState, key + 8, state ? XKB_KEY_DOWN : XKB_KEY_UP);

    if (keyboardFocusedLmfwWindow < lmfwWindowCount && key < 139) {
        lmfwWindows[keyboardFocusedLmfwWindow].keyStates[key] = state;
    }
}

void lmfw_wl_keyboard_modifiers(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {

}

void lmfw_wl_keyboard_repeat_info(void* data, struct wl_keyboard* wl_keyboard, int32_t rate, int32_t delay) {

}

void lmfw_wl_pointer_enter(void *data, struct wl_pointer* wl_pointer, uint32_t serial, struct wl_surface* surface, wl_fixed_t surface_x, wl_fixed_t surface_y) {
    for (uint32_t i = 0; i < lmfwWindowCount; i++) {
        if (lmfwWindows[i].platformWindowData.wlWindowData->wlSurface == surface) {
            pointerFocusedLmfwWindow = i;
        }
    }
}

void lmfw_wl_pointer_leave(void *data, struct wl_pointer* wl_pointer, uint32_t serial, struct wl_surface* surface) {
    for (uint32_t i = 0; i < lmfwWindowCount; i++) {
        if (lmfwWindows[i].platformWindowData.wlWindowData->wlSurface == surface && pointerFocusedLmfwWindow == i) {
            pointerFocusedLmfwWindow = LMFW_WINDOW_INVALID;
        }
    }
}

void lmfw_wl_pointer_motion(void *data, struct wl_pointer* wl_pointer, uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y) {
    if (pointerFocusedLmfwWindow >= lmfwWindowCount) return;
    if (!lmfwWindows) return;

    lmfwWindows[pointerFocusedLmfwWindow].pointerX = surface_x;
    lmfwWindows[pointerFocusedLmfwWindow].pointerY = surface_y;
}

void lmfw_wl_pointer_button(void *data, struct wl_pointer* wl_pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state) {

}

void lmfw_wl_xdg_ping(void* data, struct xdg_wm_base* shell, uint32_t serial) {
    xdg_wm_base_pong(shell, serial);
}

void lmfw_wl_registry_global(void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version) {
    if (!strcmp(interface, wl_compositor_interface.name)) {
        wlCompositor = wl_registry_bind(registry, name, &wl_compositor_interface, LMFW_WL_VERSION);
    }

    else if (!strcmp(interface, wl_shm_interface.name)) {
        wlSharedMemory = wl_registry_bind(registry, name, &wl_shm_interface, version);
    }

    else if (!strcmp(interface, xdg_wm_base_interface.name)) {
        xdgShell = wl_registry_bind(registry, name, &xdg_wm_base_interface, version);
        xdg_wm_base_add_listener(xdgShell, &shListener, NULL);
    }
    else if (!strcmp(interface, zxdg_decoration_manager_v1_interface.name)) {
        xdgDecorationManager = wl_registry_bind(registry, name, &zxdg_decoration_manager_v1_interface, 1);
    }
    else if (!strcmp(interface, wl_seat_interface.name)) {
        wlSeat = wl_registry_bind(registry, name, &wl_seat_interface, 1);
        wl_seat_add_listener(wlSeat, &wlSeatListener, NULL);
    }
}

void lmfw_wl_registry_global_remove(void* data, struct wl_registry* registry, uint32_t name) {

}
#endif

#ifdef LMFW_XCB

bool xcbInit = false;
xcb_connection_t* xcbConnection;
xcb_screen_t* xcbScreen;

void lmfw_xcb_init() {
    xcbConnection = xcb_connect(NULL, NULL);
    if (xcb_connection_has_error(xcbConnection)) {
        return;
    }

    xcbScreen = xcb_setup_roots_iterator(xcb_get_setup(xcbConnection)).data;

    xcbInit = true;
}



lmfw_window lmfw_xcb_create_window(int16_t x, int16_t y, uint16_t width, uint16_t height, const char* title, lmfw_window_type windowType, bool inputPassthrough) {
    if (!xcbInit) {
        return LMFW_WINDOW_INVALID;
    }

    lmfwWindowCount++;

    uint32_t oldLmfwWindowCapacity = lmfwWindowCapacity;
    if (lmfwWindowCount >= lmfwWindowCapacity) {
        lmfwWindowCapacity = lmfwWindowCapacity ? lmfwWindowCapacity * 2 : 1;
        lmfwWindows = realloc(lmfwWindows, lmfwWindowCapacity * sizeof(lmfw_window_data));

        memset(&lmfwWindows[oldLmfwWindowCapacity], 0, (lmfwWindowCapacity - oldLmfwWindowCapacity) * sizeof(lmfw_window_data));
    }

    lmfw_window window = lmfwWindowCount - 1;
    lmfw_window_data* windowData = &lmfwWindows[window];

    windowData->width = width;
    windowData->height = height;
    windowData->windowType = (windowType & 0b00000001) | LMFW_WINDOW_TYPE_WINDOW_BACKEND_XCB;
    windowData->platformWindowData.xcbWindowData = malloc(sizeof(lmfw_xcb_window_data));
    windowData->open = false;

    windowData->platformWindowData.xcbWindowData->xcbWindow = xcb_generate_id(xcbConnection);

    uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    uint32_t values[2] = {xcbScreen->white_pixel, XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_POINTER_MOTION};

    xcb_create_window(
        xcbConnection,
        XCB_COPY_FROM_PARENT,
        windowData->platformWindowData.xcbWindowData->xcbWindow,
        xcbScreen->root,
        x, y,
        width, height,
        0,
        XCB_WINDOW_CLASS_INPUT_OUTPUT,
        xcbScreen->root_visual,
        mask,
        values
    );

    xcb_change_property(xcbConnection, XCB_PROP_MODE_REPLACE, window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, strlen(title), title);

    xcb_map_window(xcbConnection, windowData->platformWindowData.xcbWindowData->xcbWindow);
    xcb_flush(xcbConnection);

    if (lmfw_get_graphics_backend(window) == LMFW_WINDOW_TYPE_GRAPHICS_BACKEND_SOFTWARE) {
        windowData->platformWindowData.xcbWindowData->xcbGraphicsContext = xcb_generate_id(xcbConnection);
        xcb_create_gc(xcbConnection, windowData->platformWindowData.xcbWindowData->xcbGraphicsContext, windowData->platformWindowData.xcbWindowData->xcbWindow, 0, NULL);
    }

    windowData->open = true;

    return window;
}

xcb_connection_t* lmfw_xcb_get_connection() {
    return xcbConnection;
}

xcb_window_t lmfw_xcb_get_window(lmfw_window window) {
    return lmfwWindows[window].platformWindowData.xcbWindowData->xcbWindow;
}

void lmfw_xcb_poll_events(bool blocking) {
    xcb_generic_event_t* xcbEvent;

    while ((xcbEvent = xcb_poll_for_event(xcbConnection))) {
        switch (xcbEvent->response_type & ~0x80) {
            case (XCB_CONFIGURE_NOTIFY): {
                xcb_configure_notify_event_t* xcbConfigureEvent = (xcb_configure_notify_event_t*)xcbEvent;

                lmfw_window_data* windowData;

                for (uint32_t i = 0; i < lmfwWindowCount; i++) {
                    if (lmfwWindows[i].platformWindowData.xcbWindowData->xcbWindow == xcbConfigureEvent->window) {
                        windowData = &lmfwWindows[i];
                    }
                }

                if (!windowData) break;

                windowData->width = xcbConfigureEvent->width;
                windowData->height = xcbConfigureEvent->height;
                break;
            }
            case (XCB_KEY_PRESS): {
                xcb_key_press_event_t* xcbKeyPressEvent = (xcb_key_press_event_t*)xcbEvent;

                lmfw_window_data* windowData;

                for (uint32_t i = 0; i < lmfwWindowCount; i++) {
                    if (lmfwWindows[i].platformWindowData.xcbWindowData->xcbWindow == xcbKeyPressEvent->event) {
                        windowData = &lmfwWindows[i];
                    }
                }

                if (!windowData) break;

                windowData->keyStates[xcbKeyPressEvent->detail - 8] = true;

                break;
            }
            case (XCB_KEY_RELEASE): {
                xcb_key_release_event_t* xcbKeyReleaseEvent = (xcb_key_release_event_t*)xcbEvent;

                lmfw_window_data* windowData;

                for (uint32_t i = 0; i < lmfwWindowCount; i++) {
                    if (lmfwWindows[i].platformWindowData.xcbWindowData->xcbWindow == xcbKeyReleaseEvent->event) {
                        windowData = &lmfwWindows[i];
                    }
                }

                if (!windowData) break;

                windowData->keyStates[xcbKeyReleaseEvent->detail - 8] = false;

                break;
            }
            case (XCB_MOTION_NOTIFY): {
                xcb_motion_notify_event_t* xcbMotionEvent = (xcb_motion_notify_event_t*)xcbEvent;

                lmfw_window_data* windowData;

                for (uint32_t i = 0; i < lmfwWindowCount; i++) {
                    if (lmfwWindows[i].platformWindowData.xcbWindowData->xcbWindow == xcbMotionEvent->event) {
                        windowData = &lmfwWindows[i];
                    }
                }

                if (!windowData) break;

                if (xcbMotionEvent->event_x < 0) {
                    windowData->pointerX = 0;
                } else if (xcbMotionEvent->event_x > windowData->width) {
                    windowData->pointerX = windowData->width;
                } else {
                    windowData->pointerX = xcbMotionEvent->event_x << 8;
                }

                if (xcbMotionEvent->event_y < 0) {
                    windowData->pointerY = 0;
                } else if (xcbMotionEvent->event_y > windowData->width) {
                    windowData->pointerY = windowData->width;
                } else {
                    windowData->pointerY = xcbMotionEvent->event_y << 8;
                }
                break;
            }
            default: {
                break;
            }
        }
    }
}

void lmfw_xcb_push_pixels(lmfw_window window, uint16_t width, uint16_t height, int16_t dstOffsetX, int16_t dstOffsetY, const uint8_t* pixels) {
    lmfw_window_data* windowData = &lmfwWindows[window];

    xcb_put_image(xcbConnection,
        XCB_IMAGE_FORMAT_Z_PIXMAP,
        windowData->platformWindowData.xcbWindowData->xcbWindow,
        windowData->platformWindowData.xcbWindowData->xcbGraphicsContext,
        width,
        height,
        dstOffsetX,
        dstOffsetY,
        0,
        24,
        width * height * 3,
        pixels
    );
}

void lmfw_xcb_submit_frame(lmfw_window window) {
    switch (lmfw_get_graphics_backend(window)) {
        case LMFW_WINDOW_TYPE_GRAPHICS_BACKEND_NONE: {
            break;
        }

        case LMFW_WINDOW_TYPE_GRAPHICS_BACKEND_SOFTWARE: {
            xcb_flush(xcbConnection);
            break;
        }

        default: {
            break;
        }
    }
}

void lmfw_xcb_close_window(lmfw_window window) {
    lmfw_window_data* windowData = &lmfwWindows[window];

    xcb_destroy_window(xcbConnection, windowData->platformWindowData.xcbWindowData->xcbWindow);
    xcb_free_gc(xcbConnection, windowData->platformWindowData.xcbWindowData->xcbGraphicsContext);

    xcb_flush(xcbConnection);


    switch (lmfw_get_graphics_backend(window)) {
        case LMFW_WINDOW_TYPE_GRAPHICS_BACKEND_NONE: {
            break;
        }

        case LMFW_WINDOW_TYPE_GRAPHICS_BACKEND_SOFTWARE: {
            break;
        }

        default: {
            break;
        }
    }

    free(windowData->platformWindowData.xcbWindowData);
    memset(windowData, 0, sizeof(lmfw_window_data));
}

void lmfw_xcb_shutdown() {
    xcb_disconnect(xcbConnection);
}
#endif

#ifdef LMFW_XLIB

bool xlibInit = false;
Display* xlibDisplay;
int xlibScreen;
Window xlibRootWindow;
XVisualInfo xlibVisualInfo;
XContext xlibContext;

void lmfw_xlib_init() {
    xlibDisplay = XOpenDisplay(NULL);
    xlibScreen = DefaultScreen(xlibDisplay);
    xlibRootWindow = RootWindow(xlibDisplay, xlibScreen);
    XMatchVisualInfo(xlibDisplay, xlibScreen, 24, TrueColor, &xlibVisualInfo);
    xlibContext = XUniqueContext();

    xlibInit = true;
}

lmfw_window lmfw_xlib_create_window(int16_t x, int16_t y, uint16_t width, uint16_t height, const char* title, lmfw_window_type windowType, bool inputPassthrough) {
    if (!xlibInit) {
        return LMFW_WINDOW_INVALID;
    }

    lmfwWindowCount++;

    uint32_t oldLmfwWindowCapacity = lmfwWindowCapacity;
    if (lmfwWindowCount >= lmfwWindowCapacity) {
        lmfwWindowCapacity = lmfwWindowCapacity ? lmfwWindowCapacity * 2 : 1;
        lmfwWindows = realloc(lmfwWindows, lmfwWindowCapacity * sizeof(lmfw_window_data));

        memset(&lmfwWindows[oldLmfwWindowCapacity], 0, (lmfwWindowCapacity - oldLmfwWindowCapacity) * sizeof(lmfw_window_data));
    }

    lmfw_window window = lmfwWindowCount - 1;
    lmfw_window_data* windowData = &lmfwWindows[window];

    windowData->width = width;
    windowData->height = height;
    windowData->windowType = (windowType & 0b00000001) | LMFW_WINDOW_TYPE_WINDOW_BACKEND_XLIB;
    windowData->platformWindowData.xlibWindowData = malloc(sizeof(lmfw_xlib_window_data));
    windowData->open = false;

    XSetWindowAttributes xlibWindowAttributes = {0};

    xlibWindowAttributes.colormap = XCreateColormap(
        xlibDisplay,
        xlibRootWindow,
        xlibVisualInfo.visual,
        AllocNone
    );

    xlibWindowAttributes.event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask | PointerMotionMask;

    windowData->platformWindowData.xlibWindowData->xlibWindow = XCreateWindow(
        xlibDisplay,
        xlibRootWindow,
        x,
        y,
        width,
        height,
        0,
        xlibVisualInfo.depth,
        InputOutput,
        xlibVisualInfo.visual,
        CWColormap | CWEventMask,
        &xlibWindowAttributes
    );

    XSaveContext(xlibDisplay, windowData->platformWindowData.xlibWindowData->xlibWindow, xlibContext, (char*)(uintptr_t)windowData);

    Atom net_wm_name = XInternAtom(xlibDisplay, "_NET_WM_NAME", False);
    Atom utf8_string = XInternAtom(xlibDisplay, "UTF8_STRING", False);

    XChangeProperty(
        xlibDisplay,
        windowData->platformWindowData.xlibWindowData->xlibWindow,
        net_wm_name,
        utf8_string,
        8,
        PropModeReplace,
        (unsigned char*)title,
        strlen(title)
    );

    XMapWindow(xlibDisplay, windowData->platformWindowData.xlibWindowData->xlibWindow);
    XFlush(xlibDisplay);

    if (lmfw_get_graphics_backend(window) == LMFW_WINDOW_TYPE_GRAPHICS_BACKEND_SOFTWARE) {
        XCreateGC(xlibDisplay, windowData->platformWindowData.xlibWindowData->xlibWindow, 0, NULL);
    }

    windowData->open = true;

    return window;
}

Display* lmfw_xlib_get_display() {
    return xlibDisplay;
}

Window lmfw_xlib_get_window(lmfw_window window) {
    return lmfwWindows[window].platformWindowData.xlibWindowData->xlibWindow;
}

void lmfw_xlib_poll_events(bool blocking) {
    while (XPending(xlibDisplay)) {
        XEvent xlibEvent;
        XNextEvent(xlibDisplay, &xlibEvent);

        switch (xlibEvent.type) {
            case ConfigureNotify: {
                XConfigureEvent* xlibConfigureEvent = (XConfigureEvent*)&xlibEvent;

                XPointer xlibPointer;
                XFindContext(xlibDisplay, xlibConfigureEvent->window, xlibContext, &xlibPointer);

                lmfw_window_data* windowData = (lmfw_window_data*)xlibPointer;

                windowData->width = xlibConfigureEvent->width;
                windowData->height = xlibConfigureEvent->height;

                break;
            }
            case (KeyPress): {
                XKeyPressedEvent* xlibKeyPressedEvent = (XKeyPressedEvent*)&xlibEvent;

                XPointer xlibPointer;
                XFindContext(xlibDisplay, xlibKeyPressedEvent->window, xlibContext, &xlibPointer);

                lmfw_window_data* windowData = (lmfw_window_data*)xlibPointer;

                windowData->keyStates[xlibKeyPressedEvent->keycode - 8] = true;

                break;
            }
            case (KeyRelease): {
                XKeyReleasedEvent* xlibKeyReleasedEvent = (XKeyReleasedEvent*)&xlibEvent;

                XPointer xlibPointer;
                XFindContext(xlibDisplay, xlibKeyReleasedEvent->window, xlibContext, &xlibPointer);

                lmfw_window_data* windowData = (lmfw_window_data*)xlibPointer;

                windowData->keyStates[xlibKeyReleasedEvent->keycode - 8] = false;

                break;
            }
            case (MotionNotify): {
                XMotionEvent* xlibMotionEvent = (XMotionEvent*)&xlibEvent;

                XPointer xlibPointer;
                XFindContext(xlibDisplay, xlibMotionEvent->window, xlibContext, &xlibPointer);

                lmfw_window_data* windowData = (lmfw_window_data*)xlibPointer;

                if (xlibMotionEvent->x < 0) {
                    windowData->pointerX = 0;
                } else if (xlibMotionEvent->x > windowData->width) {
                    windowData->pointerX = windowData->width;
                } else {
                    windowData->pointerX = xlibMotionEvent->x << 8;
                }

                if (xlibMotionEvent->y < 0) {
                    windowData->pointerY = 0;
                } else if (xlibMotionEvent->y > windowData->width) {
                    windowData->pointerY = windowData->width;
                } else {
                    windowData->pointerY = xlibMotionEvent->y << 8;
                }
                break;
            }
            default: {
                break;
            }
        }
    }
}

void lmfw_xlib_push_pixels(lmfw_window window, uint32_t srcOffsetX, uint32_t srcOffsetY, uint32_t dstOffsetX, uint32_t dstOffsetY, uint32_t width, uint32_t height, const uint8_t* pixels) {
    lmfw_window_data* windowData = &lmfwWindows[window];

    XImage* xlibImage = XCreateImage(
        xlibDisplay,
        xlibVisualInfo.visual,
        24,
        ZPixmap,
        0,
        (char*)pixels,
        width,
        height,
        32,
        0
    );

    XPutImage(
        xlibDisplay,
        windowData->platformWindowData.xlibWindowData->xlibWindow,
        windowData->platformWindowData.xlibWindowData->xlibGraphicsContext,
        xlibImage,
        srcOffsetX,
        srcOffsetY,
        dstOffsetX,
        dstOffsetY,
        width,
        height
    );
}

void lmfw_xlib_submit_frame(lmfw_window window) {
    switch (lmfw_get_graphics_backend(window)) {
        case LMFW_WINDOW_TYPE_GRAPHICS_BACKEND_NONE: {
            break;
        }

        case LMFW_WINDOW_TYPE_GRAPHICS_BACKEND_SOFTWARE: {
            XFlush(xlibDisplay);
            break;
        }

        default: {
            break;
        }
    }
}

void lmfw_xlib_close_window(lmfw_window window) {
    lmfw_window_data* windowData = &lmfwWindows[window];
    XDestroyWindow(xlibDisplay, windowData->platformWindowData.xlibWindowData->xlibWindow);

    XFlush(xlibDisplay);

    switch (lmfw_get_graphics_backend(window)) {
        case LMFW_WINDOW_TYPE_GRAPHICS_BACKEND_NONE: {
            break;
        }

        case LMFW_WINDOW_TYPE_GRAPHICS_BACKEND_SOFTWARE: {
            break;
        }

        default: {
            break;
        }
    }

    free(windowData->platformWindowData.xlibWindowData);
    memset(windowData, 0, sizeof(lmfw_window_data));
}

void lmfw_xlib_shutdown() {
    XCloseDisplay(xlibDisplay);
}
#endif
