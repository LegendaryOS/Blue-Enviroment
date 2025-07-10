#include "wayland_layer_shell.hpp"
#include "logger.hpp"
#include <wayland-client-protocol.h>

static void layer_surface_configure(void* data, struct zwlr_layer_surface_v1* layer_surface,
                                   uint32_t serial, uint32_t width, uint32_t height) {
    zwlr_layer_surface_v1_ack_configure(layer_surface, serial);
}

static void layer_surface_closed(void* data, struct zwlr_layer_surface_v1* layer_surface) {
    // Handle surface closure if needed
}

static const struct zwlr_layer_surface_v1_listener layer_surface_listener = {
    .configure = layer_surface_configure,
    .closed = layer_surface_closed
};

WaylandLayerShell::WaylandLayerShell(QWindow* window) : window_(window), surface_(nullptr), layer_surface_(nullptr) {
    static Logger logger;
    logger.info("Initializing Wayland layer-shell");
}

WaylandLayerShell::~WaylandLayerShell() {
    if (layer_surface_) {
        zwlr_layer_surface_v1_destroy(layer_surface_);
    }
    if (surface_) {
        wl_surface_destroy(surface_);
    }
}

void WaylandLayerShell::init() {
    static Logger logger;
    wl_display* display = wl_display_connect(nullptr);
    if (!display) {
        logger.error("Failed to connect to Wayland display");
        return;
    }

    wl_registry* registry = wl_display_get_registry(display);
    // Note: Requires Wayfire's layer-shell protocol implementation
    // Bind to zwlr_layer_shell_v1 (simplified, needs actual registry handling)
    surface_ = wl_compositor_create_surface(/* compositor */);
    layer_surface_ = zwlr_layer_shell_v1_get_layer_surface(
        /* layer_shell */,
        surface_,
        nullptr,
        ZWLR_LAYER_SHELL_V1_LAYER_TOP,
        "blue-panel"
    );

    zwlr_layer_surface_v1_set_size(layer_surface_, 0, 48); // Auto-width, 48px height
    zwlr_layer_surface_v1_set_anchor(layer_surface_, ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
                                                    ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT |
                                                    ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);
    zwlr_layer_surface_v1_set_exclusive_zone(layer_surface_, 48);
    zwlr_layer_surface_v1_add_listener(layer_surface_, &layer_surface_listener, this);
    wl_surface_commit(surface_);
}
