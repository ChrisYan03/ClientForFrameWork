#include "ClientMainWidget.h"
#include <QApplication>
#include "LogUtil.h"
#include "Common/StyleManager.h"
#include <QScreen>
#include <QGuiApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    LOG_INFO("-------------------------------Application starting1...");
    // 初始化日志系统
    LogUtil::initLogger("ClientApp");

    ClientMainWidget w;
    LOG_INFO("-------------------------------Application starting2...");
    // Initialize and apply the default theme
    StyleManager::instance()->applyTheme(StyleManager::DarkTheme);

    
    w.show();
    // 获取主屏幕并居中
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->geometry();
        QRect windowGeometry = w.frameGeometry();
        
        int x = (screenGeometry.width() - windowGeometry.width()) / 2;
        int y = (screenGeometry.height() - windowGeometry.height()) / 2;
        
        w.move(x, y);
    }
    w.raise();
    w.ClientMainInit();
    
    LOG_INFO("-------------------------------Application started suc...");
    QObject::connect(&app, &QApplication::aboutToQuit, &w, &ClientMainWidget::ClientMainQuit);
    return app.exec();
}
