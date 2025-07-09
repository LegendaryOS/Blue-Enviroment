#include <wayfire/core.hpp>
#include <wayfire/bindings.hpp>
#include <wayfire/signal-definitions.hpp>
#include <wayfire/scene-operations.hpp>
#include <wayfire/output.hpp>
#include <wayfire/util/log.hpp>
#include <cairo/cairo.h>
#include <glib.h>
#include <gio/gio.h>
#include <filesystem>
#include <algorithm>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <codecvt>
#include <locale>
#include "blue_launcher_utils.hpp"

class BlueLauncher : public wf::plugin_interface_t {
private:
    std::vector<DesktopApp> apps;
    std::vector<DesktopApp*> filtered_apps;
    std::string current_category = "all";
    bool is_visible = false;
    float animation_progress = 0.0f;
    float search_bar_scale = 1.0f;
    std::unordered_map<int, float> ripple_animations; // Animacje ripple dla aplikacji
    std::wstring search_query;
    int selected_index = -1;
    int hovered_index = -1;
    std::unique_ptr<wf::scene::floating_inner_ptr> overlay;
    wf::wl_listener_wrapper on_key_event, on_pointer_motion, on_pointer_button;
    std::unordered_map<std::string, cairo_surface_t*> icon_cache;

    class LauncherNode : public wf::scene::floating_inner_node_t {
        BlueLauncher* launcher;
    public:
        LauncherNode(BlueLauncher* launcher) : floating_inner_node_t(false), launcher(launcher) {}
        void render(const wf::render_target_t& fb, wf::geometry_t geometry, const wf::region_t& scissors) override {
            launcher->render_menu(fb, geometry);
        }
    };

public:
    void init() override {
        auto toggle_callback = [=](auto) -> bool {
            toggle_launcher();
            return true;
        };
        wf::get_core().bindings->add_key(
            wf::create_keybinding(WLR_MODIFIER_LOGO, WLR_KEY_NONE),
            &toggle_callback
        );

        on_key_event.set_callback([=](wf::signal::keyboard_event_t* ev) {
            if (is_visible) handle_keyboard_input(&ev->event);
        });
        wf::get_core().connect(&on_key_event);

        on_pointer_motion.set_callback([=](wf::signal::pointer_motion_t* ev) {
            if (is_visible) handle_pointer_motion(ev->delta_x, ev->delta_y);
        });
        wf::get_core().connect(&on_pointer_motion);

        on_pointer_button.set_callback([=](wf::signal::pointer_button_t* ev) {
            if (is_visible) handle_pointer_button(&ev->event);
        });
        wf::get_core().connect(&on_pointer_button);

        load_applications(apps);
        load_history(apps);
        update_filtered_apps();
        overlay = std::make_unique<wf::scene::floating_inner_ptr>(new LauncherNode(this));
    }

    void fini() override {
        save_history(apps);
        clear_icon_cache();
        if (is_visible) {
            wf::scene::remove_node(wf::get_core().scene(), overlay);
            is_visible = false;
        }
    }

    void toggle_launcher() {
        if (is_visible) {
            animation_progress = 1.0f;
            animate(0.0f);
            wf::scene::remove_node(wf::get_core().scene(), overlay);
            is_visible = false;
            search_query.clear();
            selected_index = -1;
            hovered_index = -1;
            ripple_animations.clear();
            save_history(apps);
        } else {
            auto output = wf::get_core().get_active_output();
            wf::scene::add_front_node(output->node_for_layer(wf::scene::layer::OVERLAY), overlay);
            is_visible = true;
            animation_progress = 0.0f;
            search_bar_scale = 1.0f;
            animate(0.0f);
            wf::get_core().set_keyboard_focus(overlay.get());
            update_filtered_apps();
        }
    }

    void update_filtered_apps() {
        filtered_apps.clear();
        std::wstring query_lower = search_query;
        std::transform(query_lower.begin(), query_lower.end(), query_lower.begin(), ::towlower);

        for (auto& app : apps) {
            bool matches_category = (current_category == "all") ||
                                   (current_category == "recent" && app.launch_count > 0) ||
                                   (current_category == "system" && app.is_system) ||
                                   (current_category == "favorites" && app.is_favorite);
            std::wstring app_name_lower = to_wstring(app.name);
            std::transform(app_name_lower.begin(), app_name_lower.end(), app_name_lower.begin(), ::towlower);
            if (matches_category && (search_query.empty() || app_name_lower.find(query_lower) != std::wstring::npos)) {
                filtered_apps.push_back(&app);
            }
        }

        std::sort(filtered_apps.begin(), filtered_apps.end(), [](const DesktopApp* a, const DesktopApp* b) {
            return a->launch_count > b->launch_count || (a->launch_count == b->launch_count && a->name < b->name);
        });

        selected_index = filtered_apps.empty() ? -1 : std::min(selected_index, static_cast<int>(filtered_apps.size()) - 1);
        hovered_index = -1;
    }

    void render_menu(const wf::render_target_t& fb, wf::geometry_t geometry) {
        cairo_t* cr = fb.cairo;
        auto dims = wf::get_core().get_output_layout()->get_current_output()->get_screen_size();

        float alpha = animation_progress;
        cairo_set_source_rgba(cr, 0.06, 0
