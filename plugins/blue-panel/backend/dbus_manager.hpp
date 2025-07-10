#include "dbus_manager.hpp"
#include "utils.hpp"
#include "logger.hpp"
#include <regex>

DBusManager::DBusManager(std::function<void(const nlohmann::json&)> callback)
    : conn_(nullptr), callback_(callback) {
    static Logger logger;
    DBusError err;
    dbus_error_init(&err);

    conn_ = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
    if (dbus_error_is_set(&err)) {
        logger.error("D-Bus connection failed: " + std::string(err.message));
        dbus_error_free(&err);
        return;
    }

    // Add signal matches
    dbus_bus_add_match(conn_,
        "type='signal',interface='org.freedesktop.NetworkManager'",
        &err);
    dbus_bus_add_match(conn_,
        "type='signal',interface='org.bluez.Adapter1'",
        &err);
    dbus_bus_add_match(conn_,
        "type='signal',interface='org.freedesktop.UPower'",
        &err);
    dbus_bus_add_match(conn_,
        "type='signal',interface='org.pulseaudio.Server'",
        &err);

    if (dbus_error_is_set(&err)) {
        logger.error("D-Bus match failed: " + std::string(err.message));
        dbus_error_free(&err);
    }

    dbus_connection_add_filter(conn_, message_handler, this, nullptr);
}

DBusManager::~DBusManager() {
    if (conn_) {
        dbus_connection_unref(conn_);
    }
}

nlohmann::json DBusManager::get_initial_status() {
    std::lock_guard<std::mutex> lock(status_mutex_);
    status_ = nlohmann::json{
        {"wifi", {{"connected", false}}},
        {"bluetooth", {{"enabled", false}}},
        {"battery", {{"percentage", 0}, {"charging", false}}},
        {"volume", {{"level", 0}}},
        {"clock", ""}
    };
    handle_wifi_update();
    handle_bluetooth_update();
    handle_battery_update();
    handle_volume_update();
    return status_;
}

void DBusManager::start_monitoring() {
    static Logger logger;
    if (!conn_) {
        logger.error("Cannot start D-Bus monitoring: no connection");
        return;
    }

    while (dbus_connection_read_write_dispatch(conn_, 0)) {
        // Non-blocking dispatch
    }
}

void DBusManager::handle_wifi_update() {
    try {
        std::string wifi_output = utils::exec("nmcli -t -f active,ssid,signal dev wifi | grep '^yes'");
        nlohmann::json wifi;
        if (!wifi_output.empty()) {
            std::regex wifi_regex("yes:([^:]+):(\\d+)");
            std::smatch match;
            if (std::regex_search(wifi_output, match, wifi_regex)) {
                wifi["connected"] = true;
                wifi["ssid"] = match[1].str();
                wifi["signal"] = std::stoi(match[2].str());
            } else {
                wifi["connected"] = false;
            }
        } else {
            wifi["connected"] = false;
        }
        update_status("wifi", wifi);
    } catch (const std::exception& e) {
        static Logger logger;
        logger.error("Error in handle_wifi_update: " + std::string(e.what()));
    }
}

void DBusManager::handle_bluetooth_update() {
    try {
        std::string bt_output = utils::exec("bluetoothctl show | grep Powered");
        nlohmann::json bt;
        bt["enabled"] = bt_output.find("Powered: yes") != std::string::npos;
        update_status("bluetooth", bt);
    } catch (const std::exception& e) {
        static Logger logger;
        logger.error("Error in handle_bluetooth_update: " + std::string(e.what()));
    }
}

void DBusManager::handle_battery_update() {
    try {
        std::string bat_output = utils::exec("upower -i /org/freedesktop/UPower/devices/battery_BAT0 | grep percentage");
        nlohmann::json battery;
        std::regex bat_regex("percentage:\\s*(\\d+)%");
        std::smatch bat_match;
        if (std::regex_search(bat_output, bat_match, bat_regex)) {
            battery["percentage"] = std::stoi(bat_match[1].str());
            battery["charging"] = utils::exec("upower -i /org/freedesktop/UPower/devices/battery_BAT0 | grep state")
                .find("charging") != std::string::npos;
        } else {
            battery["percentage"] = 0;
            battery["charging"] = false;
        }
        update_status("battery", battery);
    } catch (const std::exception& e) {
        static Logger logger;
        logger.error("Error in handle_battery_update: " + std::string(e.what()));
    }
}

void DBusManager::handle_volume_update() {
    try {
        std::string vol_output = utils::exec("pactl get-sink-volume @DEFAULT_SINK@ | grep Volume");
        nlohmann::json volume;
        std::regex vol_regex("(\\d+)%");
        std::smatch vol_match;
        if (std::regex_search(vol_output, vol_match, vol_regex)) {
            volume["level"] = std::stoi(vol_match[1].str());
        } else {
            volume["level"] = 0;
        }
        update_status("volume", volume);
    } catch (const std::exception& e) {
        static Logger logger;
        logger.error("Error in handle_volume_update: " + std::string(e.what()));
    }
}

void DBusManager::update_status(const std::string& key, const nlohmann::json& value) {
    std::lock_guard<std::mutex> lock(status_mutex_);
    status_[key] = value;
    status_["clock"] = ctime(&time(nullptr));
    callback_(status_);
}

DBusHandlerResult DBusManager::message_handler(DBusConnection* conn, DBusMessage* msg, void* user_data) {
    DBusManager* manager = static_cast<DBusManager*>(user_data);
    static Logger logger;

    const char* interface = dbus_message_get_interface(msg);
    if (!interface) return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

    if (std::string(interface) == "org.freedesktop.NetworkManager") {
        manager->handle_wifi_update();
    } else if (std::string(interface) == "org.bluez.Adapter1") {
        manager->handle_bluetooth_update();
    } else if (std::string(interface) == "org.freedesktop.UPower") {
        manager->handle_battery_update();
    } else if (std::string(interface) == "org.pulseaudio.Server") {
        manager->handle_volume_update();
    }

    return DBUS_HANDLER_RESULT_HANDLED;
}
