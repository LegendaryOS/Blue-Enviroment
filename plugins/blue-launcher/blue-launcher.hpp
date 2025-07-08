#ifndef BLUE_LAUNCHER_HPP
#define BLUE_LAUNCHER_HPP

#include <wayfire/plugin.hpp>
#include <wayfire/output.hpp>
#include <wayfire/scene.hpp>
#include <wayfire/render-manager.hpp>
#include <cairo/cairo.h>
#include <glib.h>
#include <gio/gio.h>
#include <vector>
#include <string>

struct DesktopApp {
    std::string name;
    std::string exec;
    std::string icon;
};

class BlueLauncher : public wf::plugin_interface_t {
public:
    void init() override;
    void fini() override;

private:
    std::unique_ptr<wf::scene::floating_inner_ptr> overlay;
    std::vector<DesktopApp> apps;
    std::string search_query;
    int selected_index = 0;
    bool is_visible = false;
    float animation_progress = 0.0f;

    void toggle_launcher();
    void load_applications();
    void render_menu(const wf::render_target_t& fb, wf::geometry_t geometry);
    void handle_input(wf::keyboard_event_t* ev);
    void launch_app(const std::string& exec);
    void animate(float delta);
};

#endif // BLUE_LAUNCHER_HPP
