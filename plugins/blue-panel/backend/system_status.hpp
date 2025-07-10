#ifndef SYSTEM_STATUS_HPP
#define SYSTEM_STATUS_HPP

#include <nlohmann/json.hpp>
#include "dbus_manager.hpp"

class SystemStatus {
public:
    SystemStatus(std::function<void(const nlohmann::json&)> callback);
    nlohmann::json get_status();
    void start_monitoring();

private:
    DBusManager dbus_;
};

#endif // SYSTEM_STATUS_HPP
