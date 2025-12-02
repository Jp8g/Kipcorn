#include "../include/kipcorn.h"

void frameCallback(void* data, struct wl_callback* callback, uint32_t callbackData);
void configureXdgSurface(void* data, struct xdg_surface* surface, uint32_t serial);
void toplevelConfiguration(void* data, struct xdg_toplevel* toplevel, int32_t width, int32_t height, struct wl_array* states);
void toplevelClose(void* data, struct xdg_toplevel* toplevel);
void toplevelConfigureBounds(void* data, struct xdg_toplevel* toplevel, int32_t width, int32_t height);
void toplevelWMCapabilities(void* data, struct xdg_toplevel* toplevel, struct wl_array* states);
void seatCapabilities(void* data, struct wl_seat* seat, uint32_t capabilities);
void seatName(void* data, struct wl_seat* seat, const char* name);
void keyboardKeymap(void* data, struct wl_keyboard* wl_keyboard, uint32_t format, int32_t fd, uint32_t size);
void keyboardEnter(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, struct wl_surface* surface, struct wl_array* keys);
void keyboardLeave(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, struct wl_surface* surface);
void keyboardKey(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
void keyboardModifiers(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group);
void keyboardRepeatInfo(void* data, struct wl_keyboard* wl_keyboard, int32_t rate, int32_t delay);
void xdgPing(void* data, struct xdg_wm_base* shell, uint32_t serial);
void registryGlobal(void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version);
void registryGlobalRemove(void* data, struct wl_registry* registry, uint32_t name);

struct xdg_surface_listener xdgSurfaceListener = {configureXdgSurface};
struct wl_callback_listener callbackListener = {frameCallback};
struct xdg_toplevel_listener xdgToplevelListener = {toplevelConfiguration, toplevelClose, toplevelConfigureBounds, toplevelWMCapabilities};
struct wl_seat_listener seatListener = {seatCapabilities, seatName};
struct wl_keyboard_listener keyboardListener = {keyboardKeymap, keyboardEnter, keyboardLeave, keyboardKey, keyboardModifiers, keyboardRepeatInfo};
struct xdg_wm_base_listener shListener = {xdgPing};
struct wl_registry_listener registryListener = {registryGlobal, registryGlobalRemove};

struct wl_compositor* compositor;
struct xdg_wm_base* shell;
struct zxdg_decoration_manager_v1* decorationManager;
struct wl_shm* sharedMemory;
struct xkb_context* context;

struct wl_seat* seat;
struct wl_keyboard* keyboard;

char* keymap_string;
struct xkb_keymap* keymap;
struct xkb_state* xkbState;

void AddCallbackListener(KipcornWindow* window) {
    if (window->frameCallbackPending) return;

    wl_callback_add_listener(wl_surface_frame(window->waylandSurface), &callbackListener, window);
    window->frameCallbackPending = true;
}

KipcornWindow* KipcornCreateWindow(uint32_t width, uint32_t height, const char* title, KipcornGraphicsBackend graphicsBackend, bool vsync, bool windowDecorations, bool inputPassthrough) {
    KipcornWindow* window = malloc(sizeof(KipcornWindow));

    window->width = width;
    window->height = height;
    window->graphicsBackend = graphicsBackend;
    window->windowOpen = false;
    window->frameCallbackPending = false;
    window->frameCanRender = true;
    window->vsync = vsync;

    window->display = wl_display_connect(NULL);
    window->registry = wl_display_get_registry(window->display);
    wl_registry_add_listener(window->registry, &registryListener, window);
    wl_display_roundtrip(window->display);

    window->waylandSurface = wl_compositor_create_surface(compositor);

    window->xdgSurface = xdg_wm_base_get_xdg_surface(shell, window->waylandSurface);

    xdg_surface_add_listener(window->xdgSurface, &xdgSurfaceListener, window);
    window->toplevel = xdg_surface_get_toplevel(window->xdgSurface);
    xdg_toplevel_add_listener(window->toplevel, &xdgToplevelListener, window);
    xdg_toplevel_set_title(window->toplevel, title);
    wl_surface_commit(window->waylandSurface);

    AddCallbackListener(window);

    if (windowDecorations) {
        window->decorations = zxdg_decoration_manager_v1_get_toplevel_decoration(decorationManager, window->toplevel);
        zxdg_toplevel_decoration_v1_set_mode(window->decorations, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
    }

    if (inputPassthrough) wl_surface_set_input_region(window->waylandSurface, wl_compositor_create_region(compositor));

    if (window->graphicsBackend == KIPCORN_GRAPHICS_BACKEND_OPENGL) {
        window->eglDisplay = eglGetDisplay((EGLNativeDisplayType)window->display);
        eglInitialize(window->eglDisplay, NULL, NULL);

        EGLint attribs[] = {
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
            EGL_NONE
        };

        EGLint numConfigs;
        eglChooseConfig(window->eglDisplay, attribs, &window->eglConfig, 1, &numConfigs);

        window->eglWindow = wl_egl_window_create(window->waylandSurface, window->width, window->height);
        if (!window->eglWindow) {
            fprintf(stderr, "Failed to create wl_egl_window\n");
            return NULL;
        }

        window->eglSurface = eglCreatePlatformWindowSurface(window->eglDisplay, window->eglConfig, (EGLNativeWindowType)window->eglWindow, NULL);
        if (window->eglSurface == EGL_NO_SURFACE) {
            fprintf(stderr, "Failed to create EGL surface\n");
            return NULL;
        }

        eglBindAPI(EGL_OPENGL_API);
        window->eglContext = eglCreateContext(window->eglDisplay, window->eglConfig, EGL_NO_CONTEXT, NULL);

        if (window->eglContext == EGL_NO_CONTEXT) {
            fprintf(stderr, "Failed to create EGL context\n");
            return NULL;
        }

        eglMakeCurrent(window->eglDisplay, window->eglSurface, window->eglSurface, window->eglContext);
        eglSwapInterval(window->eglDisplay, vsync ? 1 : 0);
    }

    window->windowOpen = true;

    return window;
}

void KipcornWindowSetVsync(KipcornWindow* window, bool vsync) {
    switch (window->graphicsBackend) {
        case KIPCORN_GRAPHICS_BACKEND_SOFTWARE: {
            window->vsync = vsync;
            break;
        }

        case KIPCORN_GRAPHICS_BACKEND_OPENGL: {
            eglSwapInterval(window->eglDisplay, vsync ? 1 : 0);
            window->vsync = vsync;
            break;
        }

        case KIPCORN_GRAPHICS_BACKEND_VULKAN: {
            break;
        }
    }
}

void KipcornPollEvents(KipcornWindow* window) {
    struct wl_display* display = window->display;

    if (window->vsync) {
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

bool KipcornIsKeyDown(KipcornWindow* window, KipcornKey key) {
    return window->keyStates[key];
}

void displayFrame(KipcornWindow* window) {
    switch (window->graphicsBackend) {
        case KIPCORN_GRAPHICS_BACKEND_SOFTWARE: {
            if (!window->pixels) return;

            wl_surface_attach(window->waylandSurface, window->buffer, 0, 0);
            wl_surface_damage(window->waylandSurface, 0, 0, window->width, window->height);
            wl_surface_commit(window->waylandSurface);
            break;
        }
    
        case KIPCORN_GRAPHICS_BACKEND_OPENGL: {

            eglSwapBuffers(window->eglDisplay, window->eglSurface);
            wl_surface_commit(window->waylandSurface);
            break;
        }

        case KIPCORN_GRAPHICS_BACKEND_VULKAN: {
            break;
        }
    }
}

void KipcornSubmitFrame(KipcornWindow* window) {
    if (window->vsync && !window->frameCanRender) return;

    if (window->vsync) {
        AddCallbackListener(window);
        window->frameCanRender = false;
    }

    displayFrame(window);
}

void frameCallback(void* data, struct wl_callback* callback, uint32_t callbackData) {
    KipcornWindow* window = (KipcornWindow*)data;

    window->frameCallbackPending = false;
    window->frameCanRender = true;

    wl_callback_destroy(callback);
}

void KipcornCloseWindow(KipcornWindow* window) {
    wl_seat_release(seat);
    if (window->buffer) wl_buffer_destroy(window->buffer);
    xdg_toplevel_destroy(window->toplevel);
    xdg_surface_destroy(window->xdgSurface);
    wl_surface_destroy(window->waylandSurface);
    wl_display_disconnect(window->display);
    free(window);
}

void resize(KipcornWindow* window, uint32_t width, uint32_t height) {
    switch (window->graphicsBackend) {
        case KIPCORN_GRAPHICS_BACKEND_SOFTWARE: {
            munmap(window->pixels, window->width*  window->height*  4);

            window->width = width;
            window->height = height;

            int32_t fileDescriptor = memfd_create("", MFD_CLOEXEC);
            ftruncate(fileDescriptor, width*  height*  4);
            
            window->pixels = mmap(NULL, width*  height*  4, PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, 0);

            struct wl_shm_pool* pool = wl_shm_create_pool(sharedMemory, fileDescriptor, width*  height*  4);
            window->buffer = wl_shm_pool_create_buffer(pool, 0, width, height, width*  4, WL_SHM_FORMAT_ARGB8888);
            wl_shm_pool_destroy(pool);
            close(fileDescriptor);
            break;
        }

        case KIPCORN_GRAPHICS_BACKEND_OPENGL: {
            window->width = width;
            window->height = height;

            wl_egl_window_resize(window->eglWindow, window->width, window->height, 0, 0);
            break;
        }

        case KIPCORN_GRAPHICS_BACKEND_VULKAN: {
            break;
        }
    }
}

void configureXdgSurface(void* data, struct xdg_surface* surface, uint32_t serial) {
    KipcornWindow* window = (KipcornWindow*)data;

    xdg_surface_ack_configure(surface, serial);
    if (window->graphicsBackend == KIPCORN_GRAPHICS_BACKEND_SOFTWARE && !window->pixels) {
        resize(window, window->width, window->height);
        displayFrame(window);
    }
}

void toplevelConfiguration(void* data, struct xdg_toplevel* toplevel, int32_t width, int32_t height, struct wl_array* states) {
    if (!width && !height) {
        return;
    }

    KipcornWindow* window = (KipcornWindow*)data;

    if (window->width != width || window->height != height) {
        resize(window, width, height);
    }
}

void toplevelClose(void* data, struct xdg_toplevel* toplevel) {
    ((KipcornWindow*)data)->windowOpen = false;
}

void toplevelConfigureBounds(void* data, struct xdg_toplevel* toplevel, int32_t width, int32_t height) {

}

void toplevelWMCapabilities(void* data, struct xdg_toplevel* toplevel, struct wl_array* states) {

}

void seatCapabilities(void* data, struct wl_seat* seat, uint32_t capabilities) {

}

void seatName(void* data, struct wl_seat* seat, const char* name) {

}

void keyboardKeymap(void* data, struct wl_keyboard* wl_keyboard, uint32_t format, int32_t fd, uint32_t size) {
    keymap_string = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);

    keymap = xkb_keymap_new_from_string(context, keymap_string, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
    xkbState = xkb_state_new(keymap);
    munmap(keymap_string, size);
    close(fd);
}

void keyboardEnter(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, struct wl_surface* surface, struct wl_array* keys) {
    printf("entered!\n");
}

void keyboardLeave(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, struct wl_surface* surface) {
    printf("exited!\n");
}

void keyboardKey(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
    xkb_state_update_key(xkbState, key + 8, state ? XKB_KEY_DOWN : XKB_KEY_UP);
    //xkb_keysym_t keysym = xkb_state_key_get_one_sym(xkbState, key + 8);
    
    //uint16_t size = xkb_keysym_to_utf8(keysym, NULL, 0);

    //char buf[size];
    //xkb_keysym_to_utf8(keysym, buf, size);
    
    //if (size > 0) printf("key %s %s (code %u)\n", buf, state ? "Pressed" : "Released", key);

    KipcornWindow* window = (KipcornWindow*)data;

    window->keyStates[key] = state;
}

void keyboardModifiers(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {

}

void keyboardRepeatInfo(void* data, struct wl_keyboard* wl_keyboard, int32_t rate, int32_t delay) {

}

void xdgPing(void* data, struct xdg_wm_base* shell, uint32_t serial) {
    xdg_wm_base_pong(shell, serial);
}

void registryGlobal(void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version) {
    if (!strcmp(interface, wl_compositor_interface.name)) {
        compositor = wl_registry_bind(registry, name, &wl_compositor_interface, WL_VERSION);
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
        context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
        keyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(keyboard, &keyboardListener, data);
    }
}

void registryGlobalRemove(void* data, struct wl_registry* registry, uint32_t name) {

}