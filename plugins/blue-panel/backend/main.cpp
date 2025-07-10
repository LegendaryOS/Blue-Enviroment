#include <QApplication>
#include <QWebEngineView>
#include <QWebChannel>
#include <QDesktopWidget>
#include "webview_bridge.hpp"
#include "wayland_layer_shell.hpp"
#include "logger.hpp"

int main(int argc, char *argv[]) {
    Logger logger;
    logger.info("Starting Blue Panel");

    QApplication app(argc, argv);
    QWebEngineView view;

    // Configure WebView
    view.setUrl(QUrl("file://frontend/index.html"));
    view.setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    
    // Set geometry for top bar
    QRect screen = QApplication::desktop()->screenGeometry();
    view.setFixedSize(screen.width(), 48); // 48px height
    view.move(0, 0);

    // Enable transparency
    view.setAttribute(Qt::WA_TranslucentBackground);

    // Initialize Wayland layer-shell
    WaylandLayerShell layer_shell(&view);
    layer_shell.init();

    // Bind WebView bridge
    WebviewBridge bridge(&view);
    bridge.bindFunctions();

    view.show();
    logger.info("Blue Panel initialized");

    return app.exec();
}
