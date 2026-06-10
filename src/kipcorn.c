#define KIPCORN_WL
#include <kipcorn/kipcorn.h>

kip_window_data* kipcornWindows;
uint32_t kipcornWindowCount = 0;
uint32_t kipcornWindowCapacity = 0;

kip_window keyboardFocusedKipcornWindow = KIP_WINDOW_INVALID;
kip_window pointerFocusedKipcornWindow = KIP_WINDOW_INVALID;

#ifdef KIPCORN_WL
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

#ifdef KIPCORN_XCB
#include <xcb/xcb.h>
#include <stdlib.h>
#include <string.h>
#endif

#ifdef KIPCORN_XLIB
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <stdlib.h>
#include <string.h>
#endif

#include <stdint.h>
#include <unistd.h>

kip_window_type kip_get_window_type(kip_window window) {
    return kipcornWindows[window].windowType;
}

kip_window_type kip_get_graphics_backend(kip_window window) {
    return kipcornWindows[window].windowType & 0b00000001;
}

kip_window_type kip_get_window_backend(kip_window window) {
    return kipcornWindows[window].windowType & 0b00001110;
}

uint32_t kip_get_width(kip_window window) {
    return kipcornWindows[window].width;
}

uint32_t kip_get_height(kip_window window) {
    return kipcornWindows[window].height;
}

kip_fixed_point kip_pointer_get_x(kip_window window) {
    return kipcornWindows[window].pointerX;
}

kip_fixed_point kip_pointer_get_y(kip_window window) {
    return kipcornWindows[window].pointerY;
}

uint8_t* kip_get_pixels(kip_window window) {
    return kipcornWindows[window].pixels;
}

int32_t kip_fixed_point_to_int(kip_fixed_point fixedPoint) {
    return fixedPoint >> 8;
}

int32_t kip_fixed_point_to_int_round(kip_fixed_point fixedPoint) {
    return (fixedPoint + 0b10000000) >> 8;
}

double kip_fixed_point_to_double(kip_fixed_point fixedPoint) {
    return (double)fixedPoint / 256.0;
}

bool kip_is_key_down(kip_window window, kip_key key) {
    return key < 139 && kipcornWindows[window].keyStates[key];
}

bool kip_window_is_open(kip_window window) {
    return kipcornWindows[window].open;
}

#ifdef KIPCORN_WL
void kip_wl_configure_xdg_surface(void* data, struct xdg_surface* surface, uint32_t serial);
void kip_wl_toplevel_configuration(void* data, struct xdg_toplevel* toplevel, int32_t width, int32_t height, struct wl_array* states);
void kip_wl_toplevel_close(void* data, struct xdg_toplevel* toplevel);
void kip_wl_toplevel_configure_bounds(void* data, struct xdg_toplevel* toplevel, int32_t width, int32_t height);
void kip_wl_toplevel_wm_capabilities(void* data, struct xdg_toplevel* toplevel, struct wl_array* states);
void kip_wl_seat_capabilities(void* data, struct wl_seat* seat, uint32_t capabilities);
void kip_wl_seat_name(void* data, struct wl_seat* seat, const char* name);
void kip_wl_keyboard_keymap(void* data, struct wl_keyboard* wl_keyboard, uint32_t format, int32_t fd, uint32_t size);
void kip_wl_keyboard_enter(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, struct wl_surface* surface, struct wl_array* keys);
void kip_wl_keyboard_leave(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, struct wl_surface* surface);
void kip_wl_keyboard_key(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
void kip_wl_keyboard_modifiers(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group);
void kip_wl_keyboard_repeat_info(void* data, struct wl_keyboard* wl_keyboard, int32_t rate, int32_t delay);
void kip_wl_pointer_enter(void *data, struct wl_pointer* wl_pointer, uint32_t serial, struct wl_surface* surface, wl_fixed_t surface_x, wl_fixed_t surface_y);
void kip_wl_pointer_leave(void *data, struct wl_pointer* wl_pointer, uint32_t serial, struct wl_surface* surface);
void kip_wl_pointer_motion(void *data, struct wl_pointer* wl_pointer, uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y);
void kip_wl_pointer_button(void *data, struct wl_pointer* wl_pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state);
void kip_wl_xdg_ping(void* data, struct xdg_wm_base* shell, uint32_t serial);
void kip_wl_registry_global(void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version);
void kip_wl_registry_global_remove(void* data, struct wl_registry* registry, uint32_t name);

struct xdg_surface_listener xdgSurfaceListener = {kip_wl_configure_xdg_surface};
struct xdg_toplevel_listener xdgToplevelListener = {kip_wl_toplevel_configuration, kip_wl_toplevel_close, kip_wl_toplevel_configure_bounds, kip_wl_toplevel_wm_capabilities};
struct xdg_wm_base_listener shListener = {kip_wl_xdg_ping};
struct wl_registry_listener registryListener = {kip_wl_registry_global, kip_wl_registry_global_remove};

struct wl_compositor* wlCompositor;
struct wl_display* wlDisplay;
struct wl_registry* wlRegistry;
struct xdg_wm_base* xdgShell;
struct zxdg_decoration_manager_v1* xdgDecorationManager;

struct wl_shm* wlSharedMemory;

struct wl_seat_listener wlSeatListener = {kip_wl_seat_capabilities, kip_wl_seat_name};

struct wl_keyboard_listener wlKeyboardListener = {kip_wl_keyboard_keymap, kip_wl_keyboard_enter, kip_wl_keyboard_leave, kip_wl_keyboard_key, kip_wl_keyboard_modifiers, kip_wl_keyboard_repeat_info};

struct wl_pointer_listener wlPointerListener = {kip_wl_pointer_enter, kip_wl_pointer_leave, kip_wl_pointer_motion, kip_wl_pointer_button};

struct xkb_context* xkbContext;
struct wl_seat* wlSeat;

struct wl_keyboard* wlKeyboard;
struct xkb_keymap* xkbKeymap;
struct xkb_state* xkbState;

struct wl_pointer* wlPointer;

bool wlInit = false;

void kip_wl_init() {
    wlInit = true;
    wlDisplay = wl_display_connect(NULL);
    wlRegistry = wl_display_get_registry(wlDisplay);
    wl_registry_add_listener(wlRegistry, &registryListener, NULL);
    wl_display_roundtrip(wlDisplay);
}

kip_window kip_wl_create_window(uint32_t width, uint32_t height, const char* title, kip_window_type windowType, bool windowDecorations, bool inputPassthrough) {
    if (!wlInit) {
        return KIP_WINDOW_INVALID;
    }

    kipcornWindowCount++;

    uint32_t oldKipcornWindowCapacity = kipcornWindowCapacity;
    if (kipcornWindowCount >= kipcornWindowCapacity) {
        kipcornWindowCapacity = kipcornWindowCapacity ? kipcornWindowCapacity * 2 : 1;
        kipcornWindows = realloc(kipcornWindows, kipcornWindowCapacity * sizeof(kip_window_data));

        memset(&kipcornWindows[oldKipcornWindowCapacity], 0, (kipcornWindowCapacity - oldKipcornWindowCapacity) * sizeof(kip_window_data));
    }

    kip_window window = kipcornWindowCount - 1;
    kip_window_data* windowData = &kipcornWindows[window];

    windowData->width = width;
    windowData->height = height;
    windowData->windowType = (windowType & 0b00000001) | KIP_WINDOW_TYPE_WINDOW_BACKEND_WL;
    windowData->platformWindowData.wlWindowData = malloc(sizeof(kip_wl_window_data));
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

struct wl_display* kip_wl_get_display() {
    return wlDisplay;
}

struct wl_surface* kip_wl_get_surface(kip_window window) {
    return kipcornWindows[window].platformWindowData.wlWindowData->wlSurface;
}

void kip_wl_poll_events(bool blocking) {
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

void kip_wl_submit_frame(kip_window window) {
    kip_window_data* windowData = &kipcornWindows[window];

    switch (kip_get_graphics_backend(window)) {
        case KIP_WINDOW_TYPE_GRAPHICS_BACKEND_NONE: {
            break;
        }

        case KIP_WINDOW_TYPE_GRAPHICS_BACKEND_SOFTWARE: {
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

void kip_wl_close_window(kip_window window) {
    kip_window_data* windowData = &kipcornWindows[window];

    switch (kip_get_graphics_backend(window)) {
        case KIP_WINDOW_TYPE_GRAPHICS_BACKEND_NONE: {
            break;
        }

        case KIP_WINDOW_TYPE_GRAPHICS_BACKEND_SOFTWARE: {
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
    memset(windowData, 0, sizeof(kip_window_data));
}

void kip_wl_shutdown(void) {
    wlInit = false;


    for (uint32_t i = 0; i < kipcornWindowCount; i++) {

        if (kipcornWindows[i].platformWindowData.wlWindowData == NULL || kipcornWindows[i].platformWindowData.wlWindowData->wlSurface == NULL) continue;

        kip_wl_close_window(i);
    }

    zxdg_decoration_manager_v1_destroy(xdgDecorationManager);

    wl_keyboard_release(wlKeyboard);
    wl_seat_release(wlSeat);

    xkb_state_unref(xkbState);
    xkb_keymap_unref(xkbKeymap);
    xkb_context_unref(xkbContext);

    wl_registry_destroy(wlRegistry);
    wl_display_disconnect(wlDisplay);
    free(kipcornWindows);
}

void kip_wl_resize(kip_window window, uint32_t width, uint32_t height) {
    kip_window_data* windowData = &kipcornWindows[window];

    switch (kip_get_graphics_backend(window)) {
        case KIP_WINDOW_TYPE_GRAPHICS_BACKEND_SOFTWARE: {
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

void kip_wl_configure_xdg_surface(void* data, struct xdg_surface* surface, uint32_t serial) {
    kip_window_data* windowData = &kipcornWindows[(kip_window)(uintptr_t)data];
    if (!windowData) return;

    xdg_surface_ack_configure(surface, serial);

    if ((kip_get_graphics_backend((kip_window)(uintptr_t)data)) == KIP_WINDOW_TYPE_GRAPHICS_BACKEND_SOFTWARE && !windowData->pixels) {
        kip_wl_resize((kip_window)(uintptr_t)data, windowData->width, windowData->height);
        kip_wl_submit_frame((kip_window)(uintptr_t)data);
    }
}

void kip_wl_toplevel_configuration(void* data, struct xdg_toplevel* toplevel, int32_t width, int32_t height, struct wl_array* states) {
    if (!width && !height) {
        return;
    }

    kip_window_data* windowData = &kipcornWindows[(kip_window)(uintptr_t)data];
    if (!windowData) return;

    if (windowData->width != width || windowData->height != height) {
        kip_wl_resize((kip_window)(uintptr_t)data, width, height);
    }
}

void kip_wl_toplevel_close(void* data, struct xdg_toplevel* toplevel) {
    kip_window_data* windowData = &kipcornWindows[(kip_window)(uintptr_t)data];

    windowData->open = false;
}

void kip_wl_toplevel_configure_bounds(void* data, struct xdg_toplevel* toplevel, int32_t width, int32_t height) {

}

void kip_wl_toplevel_wm_capabilities(void* data, struct xdg_toplevel* toplevel, struct wl_array* states) {

}

void kip_wl_seat_capabilities(void* data, struct wl_seat* seat, uint32_t capabilities) {
    xkbContext = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

    wlKeyboard = wl_seat_get_keyboard(seat);
    wl_keyboard_add_listener(wlKeyboard, &wlKeyboardListener, NULL);

    wlPointer = wl_seat_get_pointer(seat);
    wl_pointer_add_listener(wlPointer, &wlPointerListener, NULL);
}

void kip_wl_seat_name(void* data, struct wl_seat* seat, const char* name) {

}

void kip_wl_keyboard_keymap(void* data, struct wl_keyboard* wl_keyboard, uint32_t format, int32_t fd, uint32_t size) {
    char* keymapString = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);

    xkbKeymap = xkb_keymap_new_from_string(xkbContext, keymapString, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
    xkbState = xkb_state_new(xkbKeymap);
    munmap(keymapString, size);
    close(fd);
}

void kip_wl_keyboard_enter(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, struct wl_surface* surface, struct wl_array* keys) {
    for (uint32_t i = 0; i < kipcornWindowCount; i++) {
        if (kipcornWindows[i].platformWindowData.wlWindowData->wlSurface == surface) {
            keyboardFocusedKipcornWindow = i;
        }
    }
}

void kip_wl_keyboard_leave(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, struct wl_surface* surface) {
    for (uint32_t i = 0; i < kipcornWindowCount; i++) {
        if (kipcornWindows[i].platformWindowData.wlWindowData->wlSurface == surface && keyboardFocusedKipcornWindow == i) {
            keyboardFocusedKipcornWindow = KIP_WINDOW_INVALID;
        }
    }
}

void kip_wl_keyboard_key(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
    if (!kipcornWindows) return;

    xkb_state_update_key(xkbState, key + 8, state ? XKB_KEY_DOWN : XKB_KEY_UP);

    if (keyboardFocusedKipcornWindow < kipcornWindowCount && key < 139) {
        kipcornWindows[keyboardFocusedKipcornWindow].keyStates[key] = state;
    }
}

void kip_wl_keyboard_modifiers(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {

}

void kip_wl_keyboard_repeat_info(void* data, struct wl_keyboard* wl_keyboard, int32_t rate, int32_t delay) {

}

void kip_wl_pointer_enter(void *data, struct wl_pointer* wl_pointer, uint32_t serial, struct wl_surface* surface, wl_fixed_t surface_x, wl_fixed_t surface_y) {
    for (uint32_t i = 0; i < kipcornWindowCount; i++) {
        if (kipcornWindows[i].platformWindowData.wlWindowData->wlSurface == surface) {
            pointerFocusedKipcornWindow = i;
        }
    }
}

void kip_wl_pointer_leave(void *data, struct wl_pointer* wl_pointer, uint32_t serial, struct wl_surface* surface) {
    for (uint32_t i = 0; i < kipcornWindowCount; i++) {
        if (kipcornWindows[i].platformWindowData.wlWindowData->wlSurface == surface && pointerFocusedKipcornWindow == i) {
            pointerFocusedKipcornWindow = KIP_WINDOW_INVALID;
        }
    }
}

void kip_wl_pointer_motion(void *data, struct wl_pointer* wl_pointer, uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y) {
    if (pointerFocusedKipcornWindow >= kipcornWindowCount) return;
    if (!kipcornWindows) return;

    kipcornWindows[pointerFocusedKipcornWindow].pointerX = surface_x;
    kipcornWindows[pointerFocusedKipcornWindow].pointerY = surface_y;
}

void kip_wl_pointer_button(void *data, struct wl_pointer* wl_pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state) {

}

void kip_wl_xdg_ping(void* data, struct xdg_wm_base* shell, uint32_t serial) {
    xdg_wm_base_pong(shell, serial);
}

void kip_wl_registry_global(void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version) {
    if (!strcmp(interface, wl_compositor_interface.name)) {
        wlCompositor = wl_registry_bind(registry, name, &wl_compositor_interface, KIP_WL_VERSION);
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

void kip_wl_registry_global_remove(void* data, struct wl_registry* registry, uint32_t name) {

}
#endif

#ifdef KIPCORN_XCB

bool xcbInit = false;
xcb_connection_t* xcbConnection;
xcb_screen_t* xcbScreen;

void kip_xcb_init() {
    xcbConnection = xcb_connect(NULL, NULL);
    if (xcb_connection_has_error(xcbConnection)) {
        return;
    }

    xcbScreen = xcb_setup_roots_iterator(xcb_get_setup(xcbConnection)).data;

    xcbInit = true;
}



kip_window kip_xcb_create_window(int16_t x, int16_t y, uint16_t width, uint16_t height, const char* title, kip_window_type windowType, bool inputPassthrough) {
    if (!xcbInit) {
        return KIP_WINDOW_INVALID;
    }

    kipcornWindowCount++;

    uint32_t oldKipcornWindowCapacity = kipcornWindowCapacity;
    if (kipcornWindowCount >= kipcornWindowCapacity) {
        kipcornWindowCapacity = kipcornWindowCapacity ? kipcornWindowCapacity * 2 : 1;
        kipcornWindows = realloc(kipcornWindows, kipcornWindowCapacity * sizeof(kip_window_data));

        memset(&kipcornWindows[oldKipcornWindowCapacity], 0, (kipcornWindowCapacity - oldKipcornWindowCapacity) * sizeof(kip_window_data));
    }

    kip_window window = kipcornWindowCount - 1;
    kip_window_data* windowData = &kipcornWindows[window];

    windowData->width = width;
    windowData->height = height;
    windowData->windowType = (windowType & 0b00000001) | KIP_WINDOW_TYPE_WINDOW_BACKEND_XCB;
    windowData->platformWindowData.xcbWindowData = malloc(sizeof(kip_xcb_window_data));
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

    if (kip_get_graphics_backend(window) == KIP_WINDOW_TYPE_GRAPHICS_BACKEND_SOFTWARE) {
        windowData->platformWindowData.xcbWindowData->xcbGraphicsContext = xcb_generate_id(xcbConnection);
        xcb_create_gc(xcbConnection, windowData->platformWindowData.xcbWindowData->xcbGraphicsContext, windowData->platformWindowData.xcbWindowData->xcbWindow, 0, NULL);
    }

    windowData->open = true;

    return window;
}

xcb_connection_t* kip_xcb_get_connection() {
    return xcbConnection;
}

xcb_window_t kip_xcb_get_window(kip_window window) {
    return kipcornWindows[window].platformWindowData.xcbWindowData->xcbWindow;
}

void kip_xcb_poll_events(bool blocking) {
    xcb_generic_event_t* xcbEvent;

    while ((xcbEvent = xcb_poll_for_event(xcbConnection))) {
        switch (xcbEvent->response_type & ~0x80) {
            case (XCB_CONFIGURE_NOTIFY): {
                xcb_configure_notify_event_t* xcbConfigureEvent = (xcb_configure_notify_event_t*)xcbEvent;

                kip_window_data* windowData;

                for (uint32_t i = 0; i < kipcornWindowCount; i++) {
                    if (kipcornWindows[i].platformWindowData.xcbWindowData->xcbWindow == xcbConfigureEvent->window) {
                        windowData = &kipcornWindows[i];
                    }
                }

                if (!windowData) break;

                windowData->width = xcbConfigureEvent->width;
                windowData->height = xcbConfigureEvent->height;
                break;
            }
            case (XCB_KEY_PRESS): {
                xcb_key_press_event_t* xcbKeyPressEvent = (xcb_key_press_event_t*)xcbEvent;

                kip_window_data* windowData;

                for (uint32_t i = 0; i < kipcornWindowCount; i++) {
                    if (kipcornWindows[i].platformWindowData.xcbWindowData->xcbWindow == xcbKeyPressEvent->event) {
                        windowData = &kipcornWindows[i];
                    }
                }

                if (!windowData) break;

                windowData->keyStates[xcbKeyPressEvent->detail - 8] = true;

                break;
            }
            case (XCB_KEY_RELEASE): {
                xcb_key_release_event_t* xcbKeyReleaseEvent = (xcb_key_release_event_t*)xcbEvent;

                kip_window_data* windowData;

                for (uint32_t i = 0; i < kipcornWindowCount; i++) {
                    if (kipcornWindows[i].platformWindowData.xcbWindowData->xcbWindow == xcbKeyReleaseEvent->event) {
                        windowData = &kipcornWindows[i];
                    }
                }

                if (!windowData) break;

                windowData->keyStates[xcbKeyReleaseEvent->detail - 8] = false;

                break;
            }
            case (XCB_MOTION_NOTIFY): {
                xcb_motion_notify_event_t* xcbMotionEvent = (xcb_motion_notify_event_t*)xcbEvent;

                kip_window_data* windowData;

                for (uint32_t i = 0; i < kipcornWindowCount; i++) {
                    if (kipcornWindows[i].platformWindowData.xcbWindowData->xcbWindow == xcbMotionEvent->event) {
                        windowData = &kipcornWindows[i];
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

void kip_xcb_push_pixels(kip_window window, uint16_t width, uint16_t height, int16_t dstOffsetX, int16_t dstOffsetY, const uint8_t* pixels) {
    kip_window_data* windowData = &kipcornWindows[window];

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

void kip_xcb_submit_frame(kip_window window) {
    switch (kip_get_graphics_backend(window)) {
        case KIP_WINDOW_TYPE_GRAPHICS_BACKEND_NONE: {
            break;
        }

        case KIP_WINDOW_TYPE_GRAPHICS_BACKEND_SOFTWARE: {
            xcb_flush(xcbConnection);
            break;
        }

        default: {
            break;
        }
    }
}

void kip_xcb_close_window(kip_window window) {
    kip_window_data* windowData = &kipcornWindows[window];

    xcb_destroy_window(xcbConnection, windowData->platformWindowData.xcbWindowData->xcbWindow);
    xcb_free_gc(xcbConnection, windowData->platformWindowData.xcbWindowData->xcbGraphicsContext);

    xcb_flush(xcbConnection);


    switch (kip_get_graphics_backend(window)) {
        case KIP_WINDOW_TYPE_GRAPHICS_BACKEND_NONE: {
            break;
        }

        case KIP_WINDOW_TYPE_GRAPHICS_BACKEND_SOFTWARE: {
            break;
        }

        default: {
            break;
        }
    }

    free(windowData->platformWindowData.xcbWindowData);
    memset(windowData, 0, sizeof(kip_window_data));
}

void kip_xcb_shutdown() {
    xcb_disconnect(xcbConnection);
}
#endif

#ifdef KIPCORN_XLIB

bool xlibInit = false;
Display* xlibDisplay;
int xlibScreen;
Window xlibRootWindow;
XVisualInfo xlibVisualInfo;
XContext xlibContext;

void kip_xlib_init() {
    xlibDisplay = XOpenDisplay(NULL);
    xlibScreen = DefaultScreen(xlibDisplay);
    xlibRootWindow = RootWindow(xlibDisplay, xlibScreen);
    XMatchVisualInfo(xlibDisplay, xlibScreen, 24, TrueColor, &xlibVisualInfo);
    xlibContext = XUniqueContext();

    xlibInit = true;
}

kip_window kip_xlib_create_window(int16_t x, int16_t y, uint16_t width, uint16_t height, const char* title, kip_window_type windowType, bool inputPassthrough) {
    if (!xlibInit) {
        return KIP_WINDOW_INVALID;
    }

    kipcornWindowCount++;

    uint32_t oldKipcornWindowCapacity = kipcornWindowCapacity;
    if (kipcornWindowCount >= kipcornWindowCapacity) {
        kipcornWindowCapacity = kipcornWindowCapacity ? kipcornWindowCapacity * 2 : 1;
        kipcornWindows = realloc(kipcornWindows, kipcornWindowCapacity * sizeof(kip_window_data));

        memset(&kipcornWindows[oldKipcornWindowCapacity], 0, (kipcornWindowCapacity - oldKipcornWindowCapacity) * sizeof(kip_window_data));
    }

    kip_window window = kipcornWindowCount - 1;
    kip_window_data* windowData = &kipcornWindows[window];

    windowData->width = width;
    windowData->height = height;
    windowData->windowType = (windowType & 0b00000001) | KIP_WINDOW_TYPE_WINDOW_BACKEND_XLIB;
    windowData->platformWindowData.xlibWindowData = malloc(sizeof(kip_xlib_window_data));
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

    if (kip_get_graphics_backend(window) == KIP_WINDOW_TYPE_GRAPHICS_BACKEND_SOFTWARE) {
        XCreateGC(xlibDisplay, windowData->platformWindowData.xlibWindowData->xlibWindow, 0, NULL);
    }

    windowData->open = true;

    return window;
}

Display* kip_xlib_get_display() {
    return xlibDisplay;
}

Window kip_xlib_get_window(kip_window window) {
    return kipcornWindows[window].platformWindowData.xlibWindowData->xlibWindow;
}

void kip_xlib_poll_events(bool blocking) {
    while (XPending(xlibDisplay)) {
        XEvent xlibEvent;
        XNextEvent(xlibDisplay, &xlibEvent);

        switch (xlibEvent.type) {
            case ConfigureNotify: {
                XConfigureEvent* xlibConfigureEvent = (XConfigureEvent*)&xlibEvent;

                XPointer xlibPointer;
                XFindContext(xlibDisplay, xlibConfigureEvent->window, xlibContext, &xlibPointer);

                kip_window_data* windowData = (kip_window_data*)xlibPointer;

                windowData->width = xlibConfigureEvent->width;
                windowData->height = xlibConfigureEvent->height;

                break;
            }
            case (KeyPress): {
                XKeyPressedEvent* xlibKeyPressedEvent = (XKeyPressedEvent*)&xlibEvent;

                XPointer xlibPointer;
                XFindContext(xlibDisplay, xlibKeyPressedEvent->window, xlibContext, &xlibPointer);

                kip_window_data* windowData = (kip_window_data*)xlibPointer;

                windowData->keyStates[xlibKeyPressedEvent->keycode - 8] = true;

                break;
            }
            case (KeyRelease): {
                XKeyReleasedEvent* xlibKeyReleasedEvent = (XKeyReleasedEvent*)&xlibEvent;

                XPointer xlibPointer;
                XFindContext(xlibDisplay, xlibKeyReleasedEvent->window, xlibContext, &xlibPointer);

                kip_window_data* windowData = (kip_window_data*)xlibPointer;

                windowData->keyStates[xlibKeyReleasedEvent->keycode - 8] = false;

                break;
            }
            case (MotionNotify): {
                XMotionEvent* xlibMotionEvent = (XMotionEvent*)&xlibEvent;

                XPointer xlibPointer;
                XFindContext(xlibDisplay, xlibMotionEvent->window, xlibContext, &xlibPointer);

                kip_window_data* windowData = (kip_window_data*)xlibPointer;

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

void kip_xlib_push_pixels(kip_window window, uint32_t srcOffsetX, uint32_t srcOffsetY, uint32_t dstOffsetX, uint32_t dstOffsetY, uint32_t width, uint32_t height, const uint8_t* pixels) {
    kip_window_data* windowData = &kipcornWindows[window];

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

void kip_xlib_submit_frame(kip_window window) {
    switch (kip_get_graphics_backend(window)) {
        case KIP_WINDOW_TYPE_GRAPHICS_BACKEND_NONE: {
            break;
        }

        case KIP_WINDOW_TYPE_GRAPHICS_BACKEND_SOFTWARE: {
            XFlush(xlibDisplay);
            break;
        }

        default: {
            break;
        }
    }
}

void kip_xlib_close_window(kip_window window) {
    kip_window_data* windowData = &kipcornWindows[window];
    XDestroyWindow(xlibDisplay, windowData->platformWindowData.xlibWindowData->xlibWindow);

    XFlush(xlibDisplay);

    switch (kip_get_graphics_backend(window)) {
        case KIP_WINDOW_TYPE_GRAPHICS_BACKEND_NONE: {
            break;
        }

        case KIP_WINDOW_TYPE_GRAPHICS_BACKEND_SOFTWARE: {
            break;
        }

        default: {
            break;
        }
    }

    free(windowData->platformWindowData.xlibWindowData);
    memset(windowData, 0, sizeof(kip_window_data));
}

void kip_xlib_shutdown() {
    XCloseDisplay(xlibDisplay);
}
#endif