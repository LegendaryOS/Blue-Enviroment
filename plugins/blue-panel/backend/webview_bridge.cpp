#include "webview_bridge.hpp"
#include "app_parser.hpp"
#include "config_manager.hpp"
#include "logger.hpp"

WebviewBridge::WebviewBridge(QWebEngineView* view, QObject* parent)
    : QObject(parent), view_(view), thread_pool_(4) {
    static Logger logger;
    QWebChannel* channel = new QWebChannel(view->page());
    view->page()->setWebChannel(channel);
    channel->registerObject(QStringLiteral("bridge"), this);

    system_status_ = new SystemStatus([this](const nlohmann::json& status) {
        thread_pool_.enqueue([this, status]() {
            view_->page()->runJavaScript(
                QString("updateStatus(%1)").arg(QString::fromStdString(status.dump()))
            );
        });
    });

    logger.info("WebviewBridge initialized");
}

void WebviewBridge::bindFunctions() {
    static Logger logger;
    logger.info("WebviewBridge functions bound");
    system_status_->start_monitoring();
}

QString WebviewBridge::get_status() {
    static Logger logger;
    try {
        return QString::fromStdString(system_status_->get_status().dump());
    } catch (const std::exception& e) {
        logger.error("Error in get_status: " + std::string(e.what()));
        return "{}";
    }
}

void WebviewBridge::launch_app(const QString& exec) {
    static Logger logger;
    thread_pool_.enqueue([exec, &logger]() {
        std::string cmd = exec.toStdString();
        if (!utils::sanitize_exec(cmd)) {
            logger.error("Invalid exec command: " + cmd);
            return;
        }
        try {
            system((cmd + " &").c_str());
            logger.info("Launched app: " + cmd);
        } catch (const std::exception& e) {
            logger.error("Error launching app: " + std::string(e.what()));
        }
    });
}

QString WebviewBridge::get_apps() {
    static Logger logger;
    try {
        return QString::fromStdString(get_apps().dump());
    } catch (const std::exception& e) {
        logger.error("Error in get_apps: " + std::string(e.what()));
        return "[]";
    }
}

QString WebviewBridge::get_pinned_apps() {
    static Logger logger;
    try {
        return QString::fromStdString(get_pinned_apps().dump());
    } catch (const std::exception& e) {
        logger.error("Error in get_pinned_apps: " + std::string(e.what()));
        return "[]";
    }
}

void WebviewBridge::set_pinned_apps(const QString& pinned) {
    static Logger logger;
    thread_pool_.enqueue([pinned, &logger]() {
        try {
            set_pinned_apps(nlohmann::json::parse(pinned.toStdString()));
            logger.info("Updated pinned apps");
        } catch (const std::exception& e) {
            logger.error("Error in set_pinned_apps: " + std::string(e.what()));
        }
    });
}

QString WebviewBridge::get_config() {
    static Logger logger;
    try {
        return QString::fromStdString(load_config().dump());
    } catch (const std::exception& e) {
        logger.error("Error in get_config: " + std::string(e.what()));
        return "{}";
    }
}

void WebviewBridge::set_config(const QString& config) {
    static Logger logger;
    thread_pool_.enqueue([config, &logger]() {
        try {
            nlohmann::json json_config = nlohmann::json::parse(config.toStdString());
            save_config(json_config);
            logger.info("Updated config");
        } catch (const std::exception& e) {
            logger.error("Error in set_config: " + std::string(e.what()));
        }
    });
}
