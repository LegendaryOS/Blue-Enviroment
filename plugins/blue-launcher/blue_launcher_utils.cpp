#include "blue_launcher_utils.hpp"
#include <filesystem>
#include <nlohmann/json.hpp>
#include <gio/gio.h>
#include <fstream>
#include <wayfire/util/log.hpp>

void load_applications(std::vector<DesktopApp>& apps) {
    std::string path = "/usr/share/applications/";
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (entry.path().extension() != ".desktop") continue;

        GKeyFile* key_file = g_key_file_new();
        if (g_key_file_load_from_file(key_file, entry.path().c_str(), G_KEY_FILE_NONE, nullptr)) {
            DesktopApp app;
            app.name = g_key_file_get_string(key_file, "Desktop Entry", "Name", nullptr) ?: "";
            app.exec = g_key_file_get_string(key_file, "Desktop Entry", "Exec", nullptr) ?: "";
            app.icon_path = resolve_icon_path(g_key_file_get_string(key_file, "Desktop Entry", "Icon", nullptr) ?: "");
            app.is_system = true;
            if (!app.name.empty() && !app.exec.empty()) {
                apps.push_back(std::move(app));
            }
        }
        g_key_file_free(key_file);
    }

    std::sort(apps.begin(), apps.end(), [](const DesktopApp& a, const DesktopApp& b) {
        return a.name < b.name;
    });
}

std::string resolve_icon_path(const std::string& icon_name) {
    if (icon_name.empty()) return "";
    if (std::filesystem::exists(icon_name)) return icon_name;

    std::vector<std::string> icon_paths = {
        "/usr/share/icons/breeze/apps/64/",
        "/usr/share/icons/breeze/apps/48/",
        "/usr/share/icons/breeze/apps/32/",
        "/usr/share/pixmaps/"
    };

    for (const auto& path : icon_paths) {
        for (const auto& ext : {".png", ".svg"}) {
            std::string full_path = path + icon_name + ext;
            if (std::filesystem::exists(full_path)) return full_path;
        }
    }
    return "";
}

void load_history(std::vector<DesktopApp>& apps) {
    std::string history_file = std::string(g_get_home_dir()) + "/.config/blue_launcher_history.json";
    if (std::filesystem::exists(history_file)) {
        std::ifstream file(history_file);
        nlohmann::json j;
        file >> j;
        for (auto& app : apps) {
            if (j.contains(app.name)) {
                auto obj = j[app.name];
                app.launch_count = obj.value("count", 0);
                app.is_favorite = obj.value("favorite", false);
            }
        }
    }
}

void save_history(const std::vector<DesktopApp>& apps) {
    std::string history_file = std::string(g_get_home_dir()) + "/.config/blue_launcher_history.json";
    nlohmann::json j;
    for (const auto& app : apps) {
        if (app.launch_count > 0 || app.is_favorite) {
            j[app.name] = {
                {"count", app.launch_count},
                {"favorite", app.is_favorite}
            };
        }
    }
    std::ofstream file(history_file);
    file << j.dump(4);
}

void launch_app(const std::string& exec) {
    std::string clean_exec = exec;
    size_t pos = clean_exec.find('%');
    if (pos != std::string::npos) {
        clean_exec = clean_exec.substr(0, pos);
    }
    std::string command = clean_exec + " &";
    GError* error = nullptr;
    g_spawn_command_line_async(command.c_str(), &error);
    if (error) {
        LOGE("Failed to launch app: ", error->message);
        g_error_free(error);
    }
}

void draw_rounded_rect(cairo_t* cr, double x, double y, double width, double height, double radius) {
    cairo_new_path(cr);
    cairo_arc(cr, x + radius, y + radius, radius, M_PI, 1.5 * M_PI);
    cairo_arc(cr, x + width - radius, y + radius, radius, 1.5 * M_PI, 2 * M_PI);
    cairo_arc(cr, x + width - radius, y + height - radius, radius, 0, 0.5 * M_PI);
    cairo_arc(cr, x + radius, y + height - radius, radius, 0.5 * M_PI, M_PI);
    cairo_close_path(cr);
}

std::wstring to_wstring(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(str);
}

std::string to_string(const std::wstring& wstr) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wstr);
}
