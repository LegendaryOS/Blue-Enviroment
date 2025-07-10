#ifndef WAYLAND_LAYER_SHELL_HPP
#define WAYLAND_LAYER_SHELL_HPP

#include <QWindow>
#include <wayland-client.h>
#include <wayland-layer-shell.h>

class WaylandLayerShell {
public:
    WaylandLayerShell(QWindow* window);
    ~WaylandLayerShell();
    void init();

private:
    QWindow* window_;
    wl_surface* surface_;
    zwlr_layer_surface_v1* layer_surface_;
};

#endif // WAYLAND_LAYER_SHELL_HPP
