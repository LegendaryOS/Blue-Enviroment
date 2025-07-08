#include "blue-launcher.hpp"
#include <wayfire/core.hpp>
#include <wayfire/bindings.hpp>
#include <wayfire/signal-definitions.hpp>
#include <wayfire/scene-operations.hpp>
#include <wayfire/util/log.hpp>
#include <cairo/cairo.h>
#include <glib.h>
#include <gio/gio.h>
#include <filesystem>
#include <algorithm>

class LauncherNode : public wf::scene::floating_inner_node_t {
    BlueLauncher* launcher;
public:
    LauncherNode(BlueLauncher* launcher) : floating_inner_node_t(false), launcher(launcher) {}

    void render(const wf::render_target_t& fb, wf::geometry_t geometry, const wf::region_t& scissors) override {
        launcher->render_menu(fb, geometry);
    }
};

void BlueLauncher::init() {
    // Rejestracja skrótu MOD4+SPACE
    auto toggle_callback = [=](auto) -> bool {
        toggle_launcher();
        return true;
    };
    wf::get_core().bindings->add_key(
        wf::create_keybinding(WLR_MODIFIER_LOGO, WLR_KEY_SPACE),
        &toggle_callback
    );

    // Rejestracja obsługi klawiatury
    wf::get_core().connect(&on_key_event);
    on_key_event.set_callback([=](wf::signal::keyboard_event_t* ev) {
        if (is_visible) handle_input(&ev->event);
    });

    // Wczytaj aplikacje
    load_applications();

    // Inicjalizacja nakładki
    overlay = std::make_unique<wf::scene::floating_inner_ptr>(new LauncherNode(this));
}

void BlueLauncher::fini() {
    // Usuń nakładkę
    if (is_visible) {
        wf::scene::remove_node(wf::get_core().scene(), overlay);
        is_visible = false;
    }
    // Usuń bindingi
    wf::get_core().bindings->rem_key(&toggle_callback);
}

void BlueLauncher::load_applications() {
    // Katalogi do przeszukania
    std::vector<std::string> paths = {
        "/usr/share/applications/",
        std::string(g_get_home_dir()) + "/.local/share/applications/"
    };

    for (const auto& path : paths) {
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (entry.path().extension() != ".desktop") continue;

            GKeyFile* key_file = g_key_file_new();
            if (g_key_file_load_from_file(key_file, entry.path().c_str(), G_KEY_FILE_NONE, nullptr)) {
                DesktopApp app;
                app.name = g_key_file_get_string(key_file, "Desktop Entry", "Name", nullptr);
                app.exec = g_key_file_get_string(key_file, "Desktop Entry", "Exec", nullptr);
                app.icon = g_key_file_get_string(key_file, "Desktop Entry", "Icon", nullptr);
                if (!app.name.empty() && !app.exec.empty()) {
                    apps.push_back(app);
                }
            }
            g_key_file_free(key_file);
        }
    }
    std::sort(apps.begin(), apps.end(), [](const DesktopApp& a, const DesktopApp& b) {
        return a.name < b.name;
    });
}

void BlueLauncher::toggle_launcher() {
    if (is_visible) {
        // Rozpocznij animację znikania
        animation_progress = 1.0f;
        animate(0.0f);
        wf::scene::remove_node(wf::get_core().scene(), overlay);
        is_visible = false;
    } else {
        // Dodaj nakładkę do sceny
        auto output = wf::get_core().get_active_output();
        wf::scene::add_front_node(output->node_for_layer(wf::scene::layer::OVERLAY), overlay);
        is_visible = true;
        animation_progress = 0.0f;
        animate(0.0f);
        wf::get_core().set_keyboard_focus(overlay.get());
    }
}

void BlueLauncher::render_menu(const wf::render_target_t& fb, wf::geometry_t geometry) {
    cairo_t* cr = fb.cairo;
    auto dims = wf::get_core().get_output_layout()->get_current_output()->get_screen_size();

    // Tło z animacją przezroczystości
    float alpha = animation_progress;
    cairo_set_source_rgba(cr, 0.2, 0.2, 0.2, 0.8 * alpha);
    cairo_rectangle(cr, 0, 0, dims.width, dims.height);
    cairo_fill(cr);

    // Okno menu (środek ekranu, zaokrąglone krawędzie)
    int menu_width = 600, menu_height = 400;
    int menu_x = (dims.width - menu_width) / 2;
    int menu_y = (dims.height - menu_height) / 2;
    cairo_set_source_rgba(cr, 0.1, 0.1, 0.1, 0.9 * alpha);
    cairo_rectangle_rounded(cr, menu_x, menu_y, menu_width, menu_height, 10);
    cairo_fill(cr);

    // Pole wyszukiwania
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, alpha);
    cairo_move_to(cr, menu_x + 20, menu_y + 40);
    cairo_show_text(cr, ("Search: " + search_query).c_str());

    // Lista aplikacji
    int y_offset = menu_y + 80;
    int index = 0;
    for (const auto& app : apps) {
        if (!search_query.empty() && app.name.find(search_query) == std::string::npos) continue;

        // Podświetlenie wybranej aplikacji
        if (index == selected_index) {
            cairo_set_source_rgba(cr, 0.3, 0.5, 0.8, alpha);
            cairo_rectangle(cr, menu_x + 10, y_offset - 5, menu_width - 20, 30);
            cairo_fill(cr);
        }

        // Nazwa aplikacji
        cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, alpha);
        cairo_move_to(cr, menu_x + 20, y_offset + 20);
        cairo_show_text(cr, app.name.c_str());
        y_offset += 40;
        index++;
    }

    // Aktualizuj animację
    animate(0.016f); // ~60 FPS
    wf::get_core().get_active_output()->render->schedule_redraw();
}

void BlueLauncher::handle_input(wf::keyboard_event_t* ev) {
    if (ev->state == WLR_KEY_PRESSED) {
        if (ev->keyval == KEY_ESCAPE) {
            toggle_launcher();
        } else if (ev->keyval == KEY_ENTER) {
            int index = 0;
            for (const auto& app : apps) {
                if (!search_query.empty() && app.name.find(search_query) == std::string::npos) continue;
                if (index == selected_index) {
                    launch_app(app.exec);
                    toggle_launcher();
                    break;
                }
                index++;
            }
        } else if (ev->keyval == KEY_UP) {
            selected_index = std::max(0, selected_index - 1);
        } else if (ev->keyval == KEY_DOWN) {
            selected_index = std::min(selected_index + 1, (int)apps.size() - 1);
        } else if (ev->keyval >= KEY_A && ev->keyval <= KEY_Z) {
            search_query += (char)(ev->keyval - KEY_A + 'a');
        } else if (ev->keyval == KEY_BACKSPACE && !search_query.empty()) {
            search_query.pop_back();
        }
    }
}

void BlueLauncher::launch_app(const std::string& exec) {
    // Usuń parametry typu %U z Exec
    std::string clean_exec = exec;
    size_t pos = clean_exec.find('%');
    if (pos != std::string::npos) clean_exec = clean_exec.substr(0, pos);
    system((clean_exec + " &").c_str());
}

void BlueLauncher::animate(float delta) {
    if (is_visible) {
        animation_progress = std::min(animation_progress + delta * 4.0f, 1.0f); // Fade-in
    } else {
        animation_progress = std::max(animation_progress - delta * 4.0f, 0.0f); // Fade-out
    }
}

DECLARE_WAYFIRE_PLUGIN(BlueLauncher);
