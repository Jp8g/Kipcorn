#include <kipcorn/kipcorn.h>
#include <EGL/egl.h>
#include <stdint.h>
#include <wayland-client-protocol.h>
#include <wayland-util.h>
#include <xkbcommon/xkbcommon.h>

#define KIPCORN_ENABLE_INPUT 1

void FrameCallback(void* data, struct wl_callback* callback, uint32_t callbackData);
void ConfigureXdgSurface(void* data, struct xdg_surface* surface, uint32_t serial);
void ToplevelConfiguration(void* data, struct xdg_toplevel* toplevel, int32_t width, int32_t height, struct wl_array* states);
void ToplevelClose(void* data, struct xdg_toplevel* toplevel);
void ToplevelConfigureBounds(void* data, struct xdg_toplevel* toplevel, int32_t width, int32_t height);
void ToplevelWMCapabilities(void* data, struct xdg_toplevel* toplevel, struct wl_array* states);
void SeatCapabilities(void* data, struct wl_seat* seat, uint32_t capabilities);
void SeatName(void* data, struct wl_seat* seat, const char* name);
void KeyboardKeymap(void* data, struct wl_keyboard* wl_keyboard, uint32_t format, int32_t fd, uint32_t size);
void KeyboardEnter(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, struct wl_surface* surface, struct wl_array* keys);
void KeyboardLeave(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, struct wl_surface* surface);
void KeyboardKey(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
void KeyboardModifiers(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group);
void KeyboardRepeatInfo(void* data, struct wl_keyboard* wl_keyboard, int32_t rate, int32_t delay);
void PointerEnter(void *data, struct wl_pointer* wl_pointer, uint32_t serial, struct wl_surface* surface, wl_fixed_t surface_x, wl_fixed_t surface_y);
void PointerLeave(void *data, struct wl_pointer* wl_pointer, uint32_t serial, struct wl_surface* surface);
void PointerMotion(void *data, struct wl_pointer* wl_pointer, uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y);
void PointerButton(void *data, struct wl_pointer* wl_pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state);
void XdgPing(void* data, struct xdg_wm_base* shell, uint32_t serial);
void RegistryGlobal(void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version);
void RegistryGlobalRemove(void* data, struct wl_registry* registry, uint32_t name);

struct xdg_surface_listener xdgSurfaceListener = {ConfigureXdgSurface};
struct wl_callback_listener callbackListener = {FrameCallback};
struct xdg_toplevel_listener xdgToplevelListener = {ToplevelConfiguration, ToplevelClose, ToplevelConfigureBounds, ToplevelWMCapabilities};
struct xdg_wm_base_listener shListener = {XdgPing};
struct wl_registry_listener registryListener = {RegistryGlobal, RegistryGlobalRemove};

struct wl_compositor* compositor;
struct wl_display* display;
struct wl_registry* registry;
struct xdg_wm_base* shell;
struct zxdg_decoration_manager_v1* decorationManager;

struct wl_shm* sharedMemory;

struct wl_seat_listener seatListener = {SeatCapabilities, SeatName};

struct wl_keyboard_listener keyboardListener = {KeyboardKeymap, KeyboardEnter, KeyboardLeave, KeyboardKey, KeyboardModifiers, KeyboardRepeatInfo};

struct wl_pointer_listener pointerListener = {PointerEnter, PointerLeave, PointerMotion, PointerButton};

struct xkb_context* context;
struct wl_seat* seat;

struct wl_keyboard* keyboard;
struct xkb_keymap* keymap;
struct xkb_state* xkbState;

struct wl_pointer* pointer;

EGLDisplay eglDisplay;
EGLConfig eglConfig;

bool kipcornInit = false;
bool eglInit = false;

KipcornWindowData* kipcornWindows;
uint32_t kipcornWindowCount = 0;
uint32_t kipcornWindowCapacity = 0;

KipcornWindow keyboardFocusedKipcornWindow = KIPCORN_WINDOW_INVALID;
KipcornWindow pointerFocusedKipcornWindow = KIPCORN_WINDOW_INVALID;

EGLContext currentEglContext = NULL;
EGLSurface currentEglSurface = NULL;

void AddCallbackListener(KipcornWindow window) {
    KipcornWindowData* windowData = &kipcornWindows[window];

    windowData->callback = wl_surface_frame(windowData->waylandSurface);
    wl_callback_add_listener(windowData->callback, &callbackListener, (void*)(uintptr_t)window);
    windowData->frameCallbackPending = true;
}

void KipcornInit() {
    kipcornInit = true;
    display = wl_display_connect(NULL);
    registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registryListener, NULL);
    wl_display_roundtrip(display);
}

void EglInit() {
    eglInit = true;

    eglBindAPI(EGL_OPENGL_API);
    eglDisplay = eglGetDisplay((EGLNativeDisplayType)display);
    eglInitialize(eglDisplay, NULL, NULL);

    EGLint attributes[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_DEPTH_SIZE, 24,
        EGL_NONE,
    };

    EGLint numConfigs;
    eglChooseConfig(eglDisplay, attributes, &eglConfig, 1, &numConfigs);
}

KipcornWindow KipcornCreateWindow(uint32_t width, uint32_t height, const char* title, KipcornGraphicsBackend graphicsBackend, bool vsync, bool windowDecorations, bool inputPassthrough, EGLContext shareContext) {
    if (!kipcornInit) KipcornInit();

    kipcornWindowCount++;

    uint32_t oldKipcornWindowCapacity = kipcornWindowCapacity;
    if (kipcornWindowCount >= kipcornWindowCapacity) {
        kipcornWindowCapacity = kipcornWindowCapacity ? kipcornWindowCapacity * 2 : 1;
        kipcornWindows = realloc(kipcornWindows, kipcornWindowCapacity * sizeof(KipcornWindowData));

        memset(&kipcornWindows[oldKipcornWindowCapacity], 0, (kipcornWindowCapacity - oldKipcornWindowCapacity) * sizeof(KipcornWindowData));
    }

    KipcornWindow window = kipcornWindowCount - 1;
    KipcornWindowData* windowData = &kipcornWindows[window];

    windowData->width = width;
    windowData->height = height;
    windowData->graphicsBackend = graphicsBackend;
    windowData->decorationsEnabled = windowDecorations;
    windowData->open = false;
    windowData->frameCallbackPending = false;
    windowData->frameCanRender = true;
    windowData->vsync = vsync;

    windowData->waylandSurface = wl_compositor_create_surface(compositor);

    windowData->xdgSurface = xdg_wm_base_get_xdg_surface(shell, windowData->waylandSurface);

    xdg_surface_add_listener(windowData->xdgSurface, &xdgSurfaceListener, (void*)(uintptr_t)window);
    windowData->toplevel = xdg_surface_get_toplevel(windowData->xdgSurface);
    xdg_toplevel_add_listener(windowData->toplevel, &xdgToplevelListener, (void*)(uintptr_t)window);
    xdg_toplevel_set_title(windowData->toplevel, title);
    wl_surface_commit(windowData->waylandSurface);

    AddCallbackListener(window);

    if (windowData->decorationsEnabled) {
        windowData->decorations = zxdg_decoration_manager_v1_get_toplevel_decoration(decorationManager, windowData->toplevel);
        zxdg_toplevel_decoration_v1_set_mode(windowData->decorations, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
    }

    if (inputPassthrough) wl_surface_set_input_region(windowData->waylandSurface, wl_compositor_create_region(compositor));

    if (windowData->graphicsBackend == KIPCORN_GRAPHICS_BACKEND_OPENGL) {
        if (!eglInit) EglInit();

        windowData->eglWindow = wl_egl_window_create(windowData->waylandSurface, windowData->width, windowData->height);
        if (!windowData->eglWindow) {
            fprintf(stderr, "Failed to create EGL window: 0x%04x\n", eglGetError());
            return KIPCORN_WINDOW_INVALID;
        }

        windowData->eglSurface = eglCreatePlatformWindowSurface(eglDisplay, eglConfig, (EGLNativeWindowType)windowData->eglWindow, NULL);
        if (windowData->eglSurface == EGL_NO_SURFACE) {
            fprintf(stderr, "Failed to create EGL surface: 0x%04x\n", eglGetError());
            return KIPCORN_WINDOW_INVALID;
        }

        windowData->eglContext = eglCreateContext(eglDisplay, eglConfig, shareContext, NULL);

        if (windowData->eglContext == EGL_NO_CONTEXT) {
            fprintf(stderr, "Failed to create EGL context: 0x%04x\n", eglGetError());
            return KIPCORN_WINDOW_INVALID;
        }

        eglMakeCurrent(eglDisplay, windowData->eglSurface, windowData->eglSurface, windowData->eglContext);
        eglSwapInterval(eglDisplay, vsync ? 1 : 0);
    }

    windowData->open = true;

    return window;
}

void KipcornWindowSetVsync(KipcornWindow window, bool vsync) {
    KipcornWindowData* windowData = &kipcornWindows[window];

    switch (windowData->graphicsBackend) {
        case KIPCORN_GRAPHICS_BACKEND_SOFTWARE: {
            windowData->vsync = vsync;
            break;
        }
        case KIPCORN_GRAPHICS_BACKEND_OPENGL: {
            eglSwapInterval(eglDisplay, vsync ? 1 : 0);
            windowData->vsync = vsync;
            break;
        }
        case KIPCORN_GRAPHICS_BACKEND_VULKAN: {
            break;
        }

        default: {
            break;
        }
    }
}

void KipcornMakeEglContextCurrent(EGLContext context) {
    if (currentEglContext == context) return;
    currentEglContext = context;
    eglMakeCurrent(eglDisplay, currentEglSurface, currentEglSurface, context);
}

void KipcornMakeEglSurfaceCurrent(KipcornWindow window) {
    KipcornWindowData* windowData = &kipcornWindows[window];

    if (currentEglSurface == windowData->eglSurface) return;

    currentEglSurface = windowData->eglSurface;
    eglMakeCurrent(eglDisplay, currentEglSurface, currentEglSurface, windowData->eglContext);
}

uint8_t* KipcornGetPixels(KipcornWindow window) {
    return window < kipcornWindowCount && kipcornWindows[window].graphicsBackend == KIPCORN_GRAPHICS_BACKEND_SOFTWARE ? kipcornWindows[window].pixels : NULL;
}

EGLContext KipcornGetEglContext(KipcornWindow window) {
    return window < kipcornWindowCount ? kipcornWindows[window].eglContext : NULL;
}

EGLSurface KipcornGetEglSurface(KipcornWindow window) {
    return window < kipcornWindowCount ? kipcornWindows[window].eglSurface : NULL;
}

uint32_t KipcornGetWindowWidth(KipcornWindow window) {
    return window < kipcornWindowCount ? kipcornWindows[window].width : 0;
}

uint32_t KipcornGetWindowHeight(KipcornWindow window) {
    return window < kipcornWindowCount ? kipcornWindows[window].height : 0;
}

KipcornFixedPoint KipcornGetPointerX(KipcornWindow window) {
    return window < kipcornWindowCount ? kipcornWindows[window].pointerX : 0;
}

KipcornFixedPoint KipcornGetPointerY(KipcornWindow window) {
    return window < kipcornWindowCount ? kipcornWindows[window].pointerY : 0;
}

int32_t KipcornFixedPointToInt(KipcornFixedPoint fixedPoint) {
    return wl_fixed_to_int(fixedPoint);
}

double KipcornFixedPointToDouble(KipcornFixedPoint fixedPoint) {
    return wl_fixed_to_double(fixedPoint);
}

void KipcornPollEvents(bool blocking) {
    if (blocking) {
        wl_display_dispatch(display);
        return;
    }

    wl_display_dispatch_pending(display);

    if (wl_display_prepare_read(display) == 0) {
        struct pollfd pfd = {
            .fd = wl_display_get_fd(display),
            .events = POLLIN
        };

        wl_display_flush(display);

        if (poll(&pfd, 1, 0) > 0 && (pfd.revents & POLLIN)) {
            wl_display_read_events(display);
        } else {
            wl_display_cancel_read(display);
        }

        wl_display_dispatch_pending(display);
    }
}

bool KipcornIsKeyDown(KipcornWindow window, KipcornKey key) {
    return key < 139 && window < kipcornWindowCount ? kipcornWindows[window].keyStates[key] : false;
}

bool KipcornIsWindowOpen(KipcornWindow window) {
    return window < kipcornWindowCount ? kipcornWindows[window].open : false;
}

void DisplayFrame(KipcornWindowData* windowData) {
    switch (windowData->graphicsBackend) {
        case KIPCORN_GRAPHICS_BACKEND_NONE: {
            break;
        }

        case KIPCORN_GRAPHICS_BACKEND_SOFTWARE: {
            if (!windowData->pixels) return;

            wl_surface_attach(windowData->waylandSurface, windowData->buffer, 0, 0);
            wl_surface_damage(windowData->waylandSurface, 0, 0, windowData->width, windowData->height);
            wl_surface_commit(windowData->waylandSurface);
            break;
        }
    
        case KIPCORN_GRAPHICS_BACKEND_OPENGL: {
            eglSwapBuffers(eglDisplay, windowData->eglSurface);
            break;
        }

        case KIPCORN_GRAPHICS_BACKEND_VULKAN: {
            break;
        }

        default: {
            break;
        }
    }
}

bool KipcornFrameCanRender(KipcornWindow window) {
    return window < kipcornWindowCount ? kipcornWindows[window].frameCanRender : false;
}

void KipcornSubmitFrame(KipcornWindow window) {
    KipcornWindowData* windowData = &kipcornWindows[window];

    if (windowData->vsync) {
        if (!windowData->frameCanRender) return;

        windowData->frameCanRender = false;
    }

    DisplayFrame(windowData);
}

void FrameCallback(void* data, struct wl_callback* callback, uint32_t callbackData) {
    KipcornWindowData* windowData = &kipcornWindows[(KipcornWindow)(uintptr_t)data];

    windowData->frameCallbackPending = false;
    windowData->frameCanRender = true;

    wl_callback_destroy(callback);
    AddCallbackListener((KipcornWindow)(uintptr_t)data);
}

void KipcornCloseWindow(KipcornWindow window) {
    KipcornWindowData* windowData = &kipcornWindows[window];

    switch (windowData->graphicsBackend) {
        case KIPCORN_GRAPHICS_BACKEND_NONE: {
            break;
        }

        case KIPCORN_GRAPHICS_BACKEND_SOFTWARE: {
            if (windowData->buffer) wl_buffer_destroy(windowData->buffer);
            break;
        }

        case KIPCORN_GRAPHICS_BACKEND_OPENGL: {
            wl_egl_window_destroy(windowData->eglWindow);
            eglDestroySurface(eglDisplay, windowData->eglSurface);
            break;
        }

        case KIPCORN_GRAPHICS_BACKEND_VULKAN: {
            break;
        }

        default: {
            break;
        }
    }

    if (windowData->decorationsEnabled) {
        zxdg_toplevel_decoration_v1_destroy(windowData->decorations);
        windowData->decorations = NULL;
    }

    xdg_toplevel_destroy(windowData->toplevel);
    xdg_surface_destroy(windowData->xdgSurface);

    if (windowData->frameCallbackPending) wl_callback_destroy(windowData->callback);
    wl_surface_destroy(windowData->waylandSurface);

    memset(&kipcornWindows[window], 0, sizeof(KipcornWindowData));
}

void KipcornShutdown(void) {
    eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    for (uint32_t i = 0; i < kipcornWindowCount; i++) {
        if (kipcornWindows[i].graphicsBackend == KIPCORN_GRAPHICS_BACKEND_OPENGL) {
            eglDestroyContext(eglDisplay, kipcornWindows[i].eglContext);
        }

        if (kipcornWindows[i].waylandSurface == 0) continue;

        KipcornCloseWindow(i);
    }

    eglTerminate(eglDisplay);

    zxdg_decoration_manager_v1_destroy(decorationManager);

    wl_keyboard_release(keyboard);
    wl_seat_release(seat);

    xkb_state_unref(xkbState);
    xkb_keymap_unref(keymap);
    xkb_context_unref(context);

    wl_display_disconnect(display);
    free(kipcornWindows);
}

void Resize(KipcornWindowData* windowData, uint32_t width, uint32_t height) {
    switch (windowData->graphicsBackend) {
        case KIPCORN_GRAPHICS_BACKEND_SOFTWARE: {
            if (windowData->pixels) munmap(windowData->pixels, windowData->width * windowData->height * 4);

            windowData->width = width;
            windowData->height = height;

            int32_t fileDescriptor = memfd_create("", MFD_CLOEXEC);
            ftruncate(fileDescriptor, width*  height*  4);
            
            windowData->pixels = mmap(NULL, width*  height*  4, PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, 0);

            struct wl_shm_pool* pool = wl_shm_create_pool(sharedMemory, fileDescriptor, width*  height*  4);
            windowData->buffer = wl_shm_pool_create_buffer(pool, 0, width, height, width*  4, WL_SHM_FORMAT_ARGB8888);
            wl_shm_pool_destroy(pool);
            close(fileDescriptor);
            break;
        }

        case KIPCORN_GRAPHICS_BACKEND_OPENGL: {
            windowData->width = width;
            windowData->height = height;

            wl_egl_window_resize(windowData->eglWindow, windowData->width, windowData->height, 0, 0);
            break;
        }

        case KIPCORN_GRAPHICS_BACKEND_VULKAN: {
            break;
        }

        default: {
            break;
        }
    }
}

void ConfigureXdgSurface(void* data, struct xdg_surface* surface, uint32_t serial) {
    KipcornWindowData* windowData = &kipcornWindows[(KipcornWindow)(uintptr_t)data];

    xdg_surface_ack_configure(surface, serial);

    if (windowData->graphicsBackend == KIPCORN_GRAPHICS_BACKEND_SOFTWARE && !windowData->pixels) {
        Resize(windowData, windowData->width, windowData->height);
        DisplayFrame(windowData);
    }
}

void ToplevelConfiguration(void* data, struct xdg_toplevel* toplevel, int32_t width, int32_t height, struct wl_array* states) {
    if (!width && !height) {
        return;
    }

    KipcornWindowData* windowData = &kipcornWindows[(KipcornWindow)(uintptr_t)data];

    if (windowData->width != width || windowData->height != height) {
        Resize(windowData, width, height);
    }
}

void ToplevelClose(void* data, struct xdg_toplevel* toplevel) {
    KipcornWindowData* windowData = &kipcornWindows[(KipcornWindow)(uintptr_t)data];

    windowData->open = false;
}

void ToplevelConfigureBounds(void* data, struct xdg_toplevel* toplevel, int32_t width, int32_t height) {

}

void ToplevelWMCapabilities(void* data, struct xdg_toplevel* toplevel, struct wl_array* states) {

}

void SeatCapabilities(void* data, struct wl_seat* seat, uint32_t capabilities) {
    context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

    keyboard = wl_seat_get_keyboard(seat);
    wl_keyboard_add_listener(keyboard, &keyboardListener, NULL);

    pointer = wl_seat_get_pointer(seat);
    wl_pointer_add_listener(pointer, &pointerListener, NULL);
}

void SeatName(void* data, struct wl_seat* seat, const char* name) {

}

void KeyboardKeymap(void* data, struct wl_keyboard* wl_keyboard, uint32_t format, int32_t fd, uint32_t size) {
    char* keymapString = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);

    keymap = xkb_keymap_new_from_string(context, keymapString, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
    xkbState = xkb_state_new(keymap);
    munmap(keymapString, size);
    close(fd);
}

void KeyboardEnter(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, struct wl_surface* surface, struct wl_array* keys) {
    for (uint32_t i = 0; i < kipcornWindowCount; i++) {
        if (kipcornWindows[i].waylandSurface == surface) {
            keyboardFocusedKipcornWindow = i;
        }
    }
}

void KeyboardLeave(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, struct wl_surface* surface) {
    for (uint32_t i = 0; i < kipcornWindowCount; i++) {
        if (kipcornWindows[i].waylandSurface == surface && keyboardFocusedKipcornWindow == i) {
            keyboardFocusedKipcornWindow = KIPCORN_WINDOW_INVALID;
        }
    }
}

void KeyboardKey(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
    if (!kipcornWindows) return;

    xkb_state_update_key(xkbState, key + 8, state ? XKB_KEY_DOWN : XKB_KEY_UP);

    if (keyboardFocusedKipcornWindow < kipcornWindowCount && key < 139) {
        kipcornWindows[keyboardFocusedKipcornWindow].keyStates[key] = state;
    }
}

void KeyboardModifiers(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {

}

void KeyboardRepeatInfo(void* data, struct wl_keyboard* wl_keyboard, int32_t rate, int32_t delay) {

}

void PointerEnter(void *data, struct wl_pointer* wl_pointer, uint32_t serial, struct wl_surface* surface, wl_fixed_t surface_x, wl_fixed_t surface_y) {
    for (uint32_t i = 0; i < kipcornWindowCount; i++) {
        if (kipcornWindows[i].waylandSurface == surface) {
            pointerFocusedKipcornWindow = i;
        }
    }
}

void PointerLeave(void *data, struct wl_pointer* wl_pointer, uint32_t serial, struct wl_surface* surface) {
    for (uint32_t i = 0; i < kipcornWindowCount; i++) {
        if (kipcornWindows[i].waylandSurface == surface && pointerFocusedKipcornWindow == i) {
            pointerFocusedKipcornWindow = KIPCORN_WINDOW_INVALID;
        }
    }
}

void PointerMotion(void *data, struct wl_pointer* wl_pointer, uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y) {
    if (pointerFocusedKipcornWindow >= kipcornWindowCount) return;
    if (!kipcornWindows) return;

    kipcornWindows[pointerFocusedKipcornWindow].pointerX = surface_x;
    kipcornWindows[pointerFocusedKipcornWindow].pointerY = surface_y;
}

void PointerButton(void *data, struct wl_pointer* wl_pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state) {

}

void XdgPing(void* data, struct xdg_wm_base* shell, uint32_t serial) {
    xdg_wm_base_pong(shell, serial);
}

void RegistryGlobal(void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version) {
    if (!strcmp(interface, wl_compositor_interface.name)) {
        compositor = wl_registry_bind(registry, name, &wl_compositor_interface, KIPCORN_WL_VERSION);
    }

    else if (!strcmp(interface, wl_shm_interface.name)) {
        sharedMemory = wl_registry_bind(registry, name, &wl_shm_interface, version);
    }

    else if (!strcmp(interface, xdg_wm_base_interface.name)) {
        shell = wl_registry_bind(registry, name, &xdg_wm_base_interface, version);
        xdg_wm_base_add_listener(shell, &shListener, NULL);
    }
    else if (!strcmp(interface, zxdg_decoration_manager_v1_interface.name)) {
        decorationManager = wl_registry_bind(registry, name, &zxdg_decoration_manager_v1_interface, 1);
    }
    else if (!strcmp(interface, wl_seat_interface.name)) {
        seat = wl_registry_bind(registry, name, &wl_seat_interface, 1);
        wl_seat_add_listener(seat, &seatListener, NULL);
    }
}

void RegistryGlobalRemove(void* data, struct wl_registry* registry, uint32_t name) {

}