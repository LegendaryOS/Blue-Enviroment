#pragma once
#include <vector>
#include <string>
#include <cairo/cairo.h>

struct DesktopApp {
    std::string name;
    std::string exec;
    std::string icon_path;
    int launch_count = 0;
    bool is_system = false;
    bool is_favorite = false;
};

void load_applications(std::vector<DesktopApp>& apps);
std::string resolve_icon_path(const std::string& icon_name);
void load_history(std::vector<DesktopApp>& apps);
void save_history(const std::vector<DesktopApp>& apps);
void launch_app(const std::string& exec);
void draw_rounded_rect(cairo_t* cr, double x, double y, double width, double height, double radius);
std::wstring to_wstring(const std::string& str);
std::string to_string(const std::wstring& wstr);
