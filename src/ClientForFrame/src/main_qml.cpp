#include "QmlBridge/AppController.h"
#include "Common/StyleManager.h"
#include "Common/ApplicationPaths.h"
#include "Common/MainWindowSetup.h"
#include "Common/CrashpadInit.h"
#include "ComponentService.h"
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QUrl>
#include <QQuickWindow>
#include <QQuickStyle>
#include <QTimer>
#include <QDir>
#include <QCoreApplication>
#include <QtGlobal>
#include "LogUtil.h"

// 全局 Qt 消息处理，将 qDebug/qInfo/qWarning/qCritical/qFatal 输出统一转发到 LogUtil(spdlog)
static void QtLogMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    const QByteArray localMsg = msg.toLocal8Bit();
    const char* file = context.file ? context.file : "";
    int line = context.line;
    const char* function = context.function ? context.function : "";

    switch (type) {
    case QtDebugMsg:
        LOG_DEBUG("{} ({}:{}:{})", localMsg.constData(), file, line, function);
        break;
    case QtInfoMsg:
        LOG_INFO("{} ({}:{}:{})", localMsg.constData(), file, line, function);
        break;
    case QtWarningMsg:
        LOG_WARN("{} ({}:{}:{})", localMsg.constData(), file, line, function);
        break;
    case QtCriticalMsg:
        LOG_ERROR("{} ({}:{}:{})", localMsg.constData(), file, line, function);
        break;
    case QtFatalMsg:
        LOG_CRITICAL("{} ({}:{}:{})", localMsg.constData(), file, line, function);
        abort();
    }
}

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
    qInstallMessageHandler(QtLogMessageHandler);
    LOG_INFO("-------------------------------Application starting (QML)...");

    // 获取应用基础路径
    ApplicationPaths paths;
    QString baseDir = paths.baseDir();

    AppController appController;
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("appController", &appController);

    // 使用 Framework ComponentService 加载组件
    ComponentService componentService;
    componentService.setBasePath(baseDir);
    if (!componentService.initialize(&engine, &appController))
        LOG_INFO("ComponentService: one or more components failed to load (see logs)");

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

    QObject::connect(&app, &QApplication::aboutToQuit, &app, [&componentService]() {
        componentService.shutdown();
    });

    LOG_INFO("-------------------------------Application started (QML).");
    return app.exec();
}
