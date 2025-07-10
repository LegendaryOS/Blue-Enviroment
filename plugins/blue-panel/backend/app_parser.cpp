#include "app_parser.hpp"
#include "utils.hpp"
#include "logger.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>

nlohmann::json get_apps() {
    static Logger logger;
    static nlohmann::json cached_apps;
    static bool cached = false;

    if (cached) {
        logger.info("Returning cached apps");
        return cached_apps;
    }

    nlohmann::json apps;
    std::vector<std::string> paths = {
        "/usr/share/applications/",
        "/home/" + utils::exec("whoami | tr -d '\n'") + "/.local/share/applications/"
    };

    try {
        for (const auto& path : paths) {
            if (!std::filesystem::exists(path)) {
                logger.info("Directory not found: " + path);
                continue;
            }

            for (const auto& entry : std::filesystem::directory_iterator(path)) {
                if (entry.path().extension() != ".desktop") continue;

                std::ifstream file(entry.path());
                if (!file.is_open()) {
                    logger.error("Failed to open: " + entry.path().string());
                    continue;
                }

                nlohmann::json app;
                std::string line;
                while (std::getline(file, line)) {
                    line = utils::trim(line);
                    if (line.find("Name=") == 0) {
                        app["name"] = line.substr(5);
                    } else if (line.find("Icon=") == 0) {
                        app["icon"] = line.substr(5) + ".png";
                    } else if (line.find("Exec=") == 0) {
                        app["exec"] = line.substr(5);
                    }
                }

                if (app.contains("name") && app.contains("icon") && app.contains("exec")) {
                    apps.push_back(app);
                } else {
                    logger.error("Invalid .desktop file: " + entry.path().string());
                }
            }
        }
        cached_apps = apps;
        cached = true;
    } catch (const std::exception& e) {
        logger.error("Error in get_apps: " + std::string(e.what()));
    }

    return apps;
}
