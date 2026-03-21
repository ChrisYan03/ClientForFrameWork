#include "QmlBridge/AppController.h"
#include "Common/StyleManager.h"
#include "Common/ApplicationPaths.h"
#include "Common/MainWindowSetup.h"
#include "Common/CrashpadInit.h"
#include "TranslationManager.h"
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
#include <QEventLoop>
#include <QtGlobal>
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

    // 初始化翻译管理器
    TranslationManager::instance()->loadSavedLanguage();

    // 获取应用基础路径
    ApplicationPaths paths;
    QString baseDir = paths.baseDir();

    AppController appController;
    // ComponentService 必须在 QQmlApplicationEngine 之前构造，使进程退出时先析构引擎（释放组件 QML），
    // 再析构 ComponentService（unload 动态库）。若顺序反了或 aboutToQuit 里提前 shutdown，
    // 会在 DLL 已卸载后仍析构 QML 对象，关闭应用时易崩溃。
    ComponentService componentService;
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("appController", &appController);

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

    // 连接翻译管理器的语言变化信号到AppController和组件服务
    QObject::connect(TranslationManager::instance(), &TranslationManager::languageChanged, &appController, [&appController, &engine, &componentService](TranslationManager::Language language) {
        // 通知所有组件语言已变化
        componentService.notifyLanguageChanged(static_cast<int>(language));
        // 触发AppController信号
        Q_EMIT appController.currentLanguageChanged();
        // 重新设置默认页面标题以应用新翻译
        appController.setPageTitle(qApp->translate("MainWindow", "小闫客户端"));
        // 重新翻译UI
        QMetaObject::invokeMethod(&appController, "retranslateUi");
        // 触发QML引擎重新翻译
        QCoreApplication::postEvent(&engine, new QEvent(QEvent::LanguageChange));
    });

    // 连接主题变化信号到ComponentService
    QObject::connect(&appController, &AppController::themeChanged, &componentService, [&componentService](int theme) {
        componentService.notifyThemeChanged(theme);
    });

    // 退出前：先收起窗口、让组件宿主 quit（不 unload DLL），再进入析构。
    // 可减轻 macOS 上 IMK 输入法（控制台 IMKCFRunLoopWakeUpReliable）与 Qt/QML 销毁顺序冲突导致的 segfault。
    QObject::connect(&app, &QApplication::aboutToQuit, &app, [&appController, root]() {
        if (QQuickWindow *w = qobject_cast<QQuickWindow *>(root))
            w->hide();
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        appController.unregisterComponentHost();
    }, Qt::DirectConnection);

    LOG_INFO("-------------------------------Application (QML) entering event loop...");
    const int code = app.exec();
    LOG_INFO("-------------------------------Application (QML) event loop exited, code={}", code);
    return code;
}
