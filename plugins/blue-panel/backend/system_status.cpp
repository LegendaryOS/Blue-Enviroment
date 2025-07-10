#include "system_status.hpp"
#include "logger.hpp"

SystemStatus::SystemStatus(std::function<void(const nlohmann::json&)> callback)
    : dbus_(callback) {
    static Logger logger;
    logger.info("SystemStatus initialized");
}

nlohmann::json SystemStatus::get_status() {
    return dbus_.get_initial_status();
}

void SystemStatus::start_monitoring() {
    dbus_.start_monitoring();
}
