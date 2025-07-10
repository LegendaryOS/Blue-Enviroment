#include "config_manager.hpp"
#include "utils.hpp"
#include "logger.hpp"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

bool validate_config(const nlohmann::json& config) {
    static Logger logger;
    try {
        if (!config.is_object()) return false;
        if (!config.contains("pinned_apps") || !config["pinned_apps"].is_array()) return false;
        if (!config.contains("settings") || !config["settings"].is_object()) return false;

        const auto& settings = config["settings"];
        if (!settings.contains("theme") || !settings["theme"].is_string()) return false;
        if (!settings.contains("opacity") || !settings["opacity"].is_number()) return false;
        if (!settings.contains("accent_color") || !settings["accent_color"].is_string()) return false;
        if (!settings.contains("show_wifi") || !settings["show_wifi"].is_boolean()) return false;
        if (!settings.contains("show_bt") || !settings["show_bt"].is_boolean()) return false;
        if (!settings.contains("show_battery") || !settings["show_battery"].is_boolean()) return false;
        if (!settings.contains("show_volume") || !settings["show_volume"].is_boolean()) return false;
        if (!settings.contains("show_clock") || !settings["show_clock"].is_boolean()) return false;

        for (const auto& app : config["pinned_apps"]) {
            if (!app.is_object() || !app.contains("name") || !app.contains("icon") || !app.contains("exec")) {
                return false;
            }
        }
        return true;
    } catch (const std::exception& e) {
        logger.error("Config validation error: " + std::string(e.what()));
        return false;
    }
}

nlohmann::json load_config() {
    static Logger logger;
    std::string config_path = "/home/" + utils::exec("whoami | tr -d '\n'") + "/.config/blue-panel/config.json";
    nlohmann::json config;

    try {
        if (fs::exists(config_path)) {
            std::ifstream file(config_path);
            if (file.is_open()) {
                config = nlohmann::json::parse(file);
                if (!validate_config(config)) {
                    logger.error("Invalid config format, resetting to default");
                    config = {};
                } else {
                    logger.info("Loaded config from: " + config_path);
                }
            } else {
                logger.error("Failed to open config: " + config_path);
            }
        }

        if (config.empty()) {
            config = {
                {"pinned_apps", nlohmann::json::array()},
                {"settings", {
                    {"theme", "dark"},
                    {"opacity", 0.9},
                    {"accent_color", "#6200ea"},
                    {"show_wifi", true},
                    {"show_bt", true},
                    {"show_battery", true},
                    {"show_volume", true},
                    {"show_clock", true}
                }}
            };
            save_config(config);
            logger.info("Created default config at: " + config_path);
        }
    } catch (const std::exception& e) {
        logger.error("Error loading config: " + std::string(e.what()));
    }

    return config;
}

void save_config(const nlohmann::json& config) {
    static Logger logger;
    std::string config_dir = "/home/" + utils::exec("whoami | tr -d '\n'") + "/.config/blue-panel";
    std::string config_path = config_dir + "/config.json";

    try {
        if (!validate_config(config)) {
            logger.error("Invalid config format, not saving");
            return;
        }
        fs::create_directories(config_dir);
        std::ofstream file(config_path);
        if (file.is_open()) {
            file << config.dump(4);
            logger.info("Saved config to: " + config_path);
        } else {
            logger.error("Failed to save config: " + config_path);
        }
    } catch (const std::exception& e) {
        logger.error("Error saving config: " + std::string(e.what()));
    }
}

nlohmann::json get_pinned_apps() {
    return load_config()["pinned_apps"];
}

void set_pinned_apps(const nlohmann::json& pinned) {
    auto config = load_config();
    config["pinned_apps"] = pinned;
    save_config(config);
}
