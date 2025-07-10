#ifndef WEBVIEW_BRIDGE_HPP
#define WEBVIEW_BRIDGE_HPP

#include <QObject>
#include <QWebEngineView>
#include <QWebChannel>
#include "system_status.hpp"
#include "thread_pool.hpp"

class WebviewBridge : public QObject {
    Q_OBJECT
public:
    explicit WebviewBridge(QWebEngineView* view, QObject* parent = nullptr);
    void bindFunctions();

public slots:
    QString get_status();
    void launch_app(const QString& exec);
    QString get_apps();
    QString get_pinned_apps();
    void set_pinned_apps(const QString& pinned);
    QString get_config();
    void set_config(const QString& config);

private:
    QWebEngineView* view_;
    SystemStatus* system_status_;
    ThreadPool thread_pool_;
};

#endif // WEBVIEW_BRIDGE_HPP
