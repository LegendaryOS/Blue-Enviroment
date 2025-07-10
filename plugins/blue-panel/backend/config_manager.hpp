#ifndef CONFIG_MANAGER_HPP
#define CONFIG_MANAGER_HPP

#include <nlohmann/json.hpp>

nlohmann::json load_config();
void save_config(const nlohmann::json& config);
nlohmann::json get_pinned_apps();
void set_pinned_apps(const nlohmann::json& pinned);
bool validate_config(const nlohmann::json& config);

#endif // CONFIG_MANAGER_HPP
