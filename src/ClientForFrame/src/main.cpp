#include "ClientMainWidget.h"
#include <QApplication>
#include "LogUtil.h"
#include "Common/StyleManager.h"
int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication app(argc, argv);
    
    // 初始化日志系统
    LogUtil::initLogger("ClientApp");
    LOG_INFO("-------------------------------Application starting...");
    // Initialize and apply the default theme
    StyleManager::instance()->applyTheme(StyleManager::DarkTheme);

    ClientMainWidget w;
    w.show();
    w.raise();
    w.ClientMainInit();
    
    QObject::connect(&app, &QApplication::aboutToQuit, &w, &ClientMainWidget::ClientMainQuit);
    return app.exec();
}
