#include "QmlBridge/AppController.h"
#include "Common/StyleManager.h"
#include "Common/ComponentLoader.h"
#include "Common/MainWindowSetup.h"
#include "Common/CrashpadInit.h"
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QUrl>
#include <QQuickWindow>
#include <QQuickStyle>
#include <QTimer>
#include <QDir>
#include <QCoreApplication>
#include "LogUtil.h"

int main(int argc, char *argv[])
{
#if defined(Q_OS_WIN)
    qputenv("QSG_RHI_BACKEND", "opengl");
#endif
    QApplication app(argc, argv);

    const QString appDir = QCoreApplication::applicationDirPath();
    Common::initializeCrashpad(appDir, appDir + QStringLiteral("/Crashpad"));

    StyleManager::instance()->applyTheme(StyleManager::LightTheme);
    QQuickStyle::setStyle("Fusion");

    QDir::setCurrent(QCoreApplication::applicationDirPath());
    QDir().mkpath("logs");

    LogUtil::initLogger("ClientApp");
    LOG_INFO("-------------------------------Application starting (QML)...");

    AppController appController;
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("appController", &appController);

    ComponentLoader componentLoader;
    const QList<ShutdownEntry> shutdownList = componentLoader.loadAllComponents(&engine, &appController);

    QObject::connect(&engine, &QQmlApplicationEngine::warnings, [](const QList<QQmlError> &warnings) {
        for (const auto &w : warnings)
            LOG_INFO("QML: {} (line {}): {}", w.url().toString().toStdString(), w.line(), w.description().toStdString());
    });

    engine.addImportPath(QStringLiteral("qrc:/qml"));

    const QUrl url(QStringLiteral("qrc:/qml/MainWindow.qml"));
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.load(url);

    if (engine.rootObjects().isEmpty()) {
        LOG_INFO("QML root object is empty, exit.");
        return -1;
    }

    QObject *root = engine.rootObjects().first();
    QQuickWindow *win = qobject_cast<QQuickWindow *>(root);
    if (win)
        MainWindowSetup::setup(win);

    QObject::connect(&appController, &AppController::requestQuit, &app, [&app, root]() {
        if (QQuickWindow *w = qobject_cast<QQuickWindow *>(root))
            w->close();
        QApplication::closeAllWindows();
        QTimer::singleShot(100, &app, &QApplication::quit);
    }, Qt::QueuedConnection);

    QObject::connect(&app, &QApplication::aboutToQuit, &app, [shutdownList]() {
        ComponentLoader::runShutdownList(shutdownList);
    });

    LOG_INFO("-------------------------------Application started (QML).");
    return app.exec();
}
