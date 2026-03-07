#include "QmlBridge/AppController.h"
#include "Common/StyleManager.h"
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QUrl>
#include <QQuickWindow>
#include <QQuickStyle>
#include <QTimer>
#include <QPainter>
#include <QBitmap>
#include <QRegion>
#include <QWindow>
#include <QDir>
#include <QCoreApplication>
#include <QtGlobal>
#include <QLibrary>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QPair>
#include "LogUtil.h"
#if defined(Q_OS_WIN)
#include <Windows.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#ifndef DWMWA_WINDOW_CORNER_PREFERENCE
#define DWMWA_WINDOW_CORNER_PREFERENCE 33
#endif
#ifndef DWMWCP_ROUND
#define DWMWCP_ROUND 2
#endif
#endif

static const int MASK_SUPERSAMPLE = 4;

static void applyWindowRoundedMask(QWindow *window, int radiusPx)
{
    if (!window) return;
#if defined(Q_OS_WIN)
    // Windows 下 setMask 使用复杂区域会导致窗口不显示或空白，仅清除 mask 保持矩形窗口
    window->setMask(QRegion());
    return;
#else
    if (radiusPx <= 0) {
        window->setMask(QRegion());
        return;
    }
    const int w = window->width();
    const int h = window->height();
    if (w <= 0 || h <= 0) return;

    const int sw = w * MASK_SUPERSAMPLE;
    const int sh = h * MASK_SUPERSAMPLE;
    const int sr = radiusPx * MASK_SUPERSAMPLE;

    QImage hi(sw, sh, QImage::Format_ARGB32);
    hi.fill(Qt::transparent);
    QPainter painter(&hi);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.setBrush(Qt::black);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(QRect(0, 0, sw, sh), sr, sr);
    painter.end();

    QImage scaled = hi.scaled(w, h, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    QImage mono(w, h, QImage::Format_Mono);
    mono.setColorCount(2);
    mono.setColor(0, 0xFFFFFFFF);
    mono.setColor(1, 0xFF000000);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int alpha = qAlpha(scaled.pixel(x, y));
            mono.setPixel(x, y, alpha > 128 ? 1 : 0);
        }
    }
    QBitmap bitmap = QBitmap::fromImage(mono);
    window->setMask(QRegion(bitmap));
#endif
}

/** 从 config/components.json 读取启用的组件 ID 列表 */
static QStringList loadEnabledComponents()
{
    QString path = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(QStringLiteral("config/components.json"));
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return QStringList();
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
    file.close();
    if (err.error != QJsonParseError::NoError || !doc.isObject())
        return QStringList();
    QJsonArray arr = doc.object().value(QStringLiteral("components")).toArray();
    QStringList list;
    for (const QJsonValue &v : arr) {
        QString id = v.toString().trimmed();
        if (!id.isEmpty())
            list.append(id);
    }
    return list;
}

/** (module, symbol) 用于退出时调用组件的 shutdown */
using ShutdownEntry = QPair<QString, QString>;

/** 加载组件 DLL 并调用 Register(engine, appController)，组件自行注册 QML 类型；shutdownList 收集 manifest 中的 shutdownModule/shutdownSymbol 供退出时调用 */
static void loadComponentDll(QQmlEngine *engine, QObject *appController, const QString &componentId, QList<ShutdownEntry> *shutdownList)
{
    if (!engine || !appController)
        return;
    QString exeDir = QDir(QCoreApplication::applicationDirPath()).absolutePath();
    QString manifestPath = exeDir + QStringLiteral("/Component/") + componentId + QStringLiteral("/meta_info/manifest.json");
    QFile mf(manifestPath);
    if (!mf.open(QIODevice::ReadOnly | QIODevice::Text)) {
        LOG_INFO("Component {}: manifest not found at {}", componentId.toStdString(), manifestPath.toStdString());
        return;
    }
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(mf.readAll(), &err);
    mf.close();
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        LOG_INFO("Component {}: invalid manifest", componentId.toStdString());
        return;
    }
    QJsonObject obj = doc.object();
    QString module = obj.value(QStringLiteral("module")).toString();
    QString registerSymbol = obj.value(QStringLiteral("registerSymbol")).toString();
    QString shutdownModule = obj.value(QStringLiteral("shutdownModule")).toString();
    QString shutdownSymbol = obj.value(QStringLiteral("shutdownSymbol")).toString();
    if (module.isEmpty() || registerSymbol.isEmpty()) {
        LOG_INFO("Component {}: missing module or registerSymbol", componentId.toStdString());
        return;
    }
    if (shutdownList && !shutdownModule.isEmpty() && !shutdownSymbol.isEmpty()) {
        QString shutdownPath = exeDir + QStringLiteral("/Component/") + componentId + QStringLiteral("/bin/") + shutdownModule;
        shutdownList->append(qMakePair(shutdownPath, shutdownSymbol));
    }
    QString dllPath = exeDir + QStringLiteral("/Component/") + componentId + QStringLiteral("/bin/") + module;
#if defined(Q_OS_WIN)
    // 将 exe 目录和组件 bin 目录加入 DLL 搜索路径，确保 PicMatchComponent 能加载其依赖（PicPlayer、PicRecognition 等）
    QString componentBin = exeDir + QStringLiteral("/Component/") + componentId + QStringLiteral("/bin");
    const DWORD SEARCH_DEFAULT = 0x00001000;  // LOAD_LIBRARY_SEARCH_DEFAULT_DIRS
    const DWORD SEARCH_USER = 0x00000200;     // LOAD_LIBRARY_SEARCH_USER_DIRS
    if (auto setDefault = reinterpret_cast<BOOLEAN (WINAPI *)(DWORD)>(::GetProcAddress(::GetModuleHandleW(L"kernel32.dll"), "SetDefaultDllDirectories"))) {
        if (setDefault(SEARCH_DEFAULT | SEARCH_USER)) {
            if (auto addDir = reinterpret_cast<DLL_DIRECTORY_COOKIE (WINAPI *)(PCWSTR)>(::GetProcAddress(::GetModuleHandleW(L"kernel32.dll"), "AddDllDirectory"))) {
                std::wstring exeW = exeDir.toStdWString();
                std::wstring binW = componentBin.toStdWString();
                addDir(exeW.c_str());
                addDir(binW.c_str());
            }
        }
    } else {
        ::SetDllDirectoryW(componentBin.toStdWString().c_str());
    }
    QLibrary lib(dllPath);
#else
    QString libPath = dllPath;
    if (!libPath.endsWith(QStringLiteral(".so")) && !libPath.endsWith(QStringLiteral(".dylib")))
        libPath += QStringLiteral(".so");
    QLibrary lib(libPath);
#endif
    if (!lib.load()) {
        LOG_INFO("Component {}: failed to load {}: {}", componentId.toStdString(), dllPath.toStdString(), lib.errorString().toStdString());
        return;
    }
    typedef void (*RegisterFunc)(QQmlEngine *, QObject *);
    auto fn = reinterpret_cast<RegisterFunc>(lib.resolve(registerSymbol.toUtf8().constData()));
    if (!fn) {
        LOG_INFO("Component {}: symbol {} not found", componentId.toStdString(), registerSymbol.toStdString());
        lib.unload();
        return;
    }
    fn(engine, appController);
    LOG_INFO("Component {}: registered", componentId.toStdString());
}

#if defined(Q_OS_WIN)
/** Windows 11+ 下启用系统原生圆角（VS Code 风格），Win10 无效果 */
static void applyWindows11RoundedCorners(QWindow *window)
{
    if (!window)
        return;
    // winId() 在 show 后才有有效 HWND
    HWND hwnd = reinterpret_cast<HWND>(window->winId());
    if (!hwnd)
        return;
    DWORD preference = DWMWCP_ROUND;
    ::DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &preference, sizeof(preference));
}
#endif

int main(int argc, char *argv[])
{
#if defined(Q_OS_WIN)
    // Windows 下 Qt6 默认可能用 D3D，部分环境会导致 QML 白屏；优先用 OpenGL 保证首帧能画出
    qputenv("QSG_RHI_BACKEND", "opengl");
#endif
    QApplication app(argc, argv);

    StyleManager::instance()->applyTheme(StyleManager::LightTheme);

    // 使用支持 background/contentItem 自定义的样式；Fusion 在 Windows 下比 Basic 更稳定，避免白屏
    QQuickStyle::setStyle("Fusion");

    // 确保日志目录存在（以 exe 所在目录为基准，便于 Release 下生成到 target/Win64/bin/Release/logs）
    QDir::setCurrent(QCoreApplication::applicationDirPath());
    QDir().mkpath("logs");

    LogUtil::initLogger("ClientApp");
    LOG_INFO("-------------------------------Application starting (QML)...");

    AppController appController;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("appController", &appController);

    // 动态加载组件 DLL，组件自行注册 QML 类型（如 PlayerHostItem）
    QList<ShutdownEntry> shutdownList;
    for (const QString &compId : loadEnabledComponents())
        loadComponentDll(&engine, &appController, compId, &shutdownList);

    // 打印 QML 警告/错误，便于排查
    QObject::connect(&engine, &QQmlApplicationEngine::warnings, [](const QList<QQmlError> &warnings) {
        for (const auto &w : warnings)
            LOG_INFO("QML: {} (line {}): {}", w.url().toString().toStdString(), w.line(), w.description().toStdString());
    });

    // 直接加载 MainWindow.qml，根对象即为 Window，便于显示；子组件 TitleBar.qml 在同目录可被解析
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
    if (win) {
        auto updateMask = [win]() {
            const bool maximized = (win->visibility() == QWindow::Maximized);
            applyWindowRoundedMask(win, maximized ? 0 : 10);
        };
        QObject::connect(win, &QWindow::widthChanged, win, updateMask);
        QObject::connect(win, &QWindow::heightChanged, win, updateMask);
#if defined(Q_OS_WIN)
        // 最大化/恢复时延后约一帧再更新 mask，与系统动画错峰，过渡更顺滑
        QObject::connect(win, &QWindow::visibilityChanged, win, [win, updateMask]() {
            QTimer::singleShot(16, win, updateMask);
        });
#else
        QObject::connect(win, &QWindow::visibilityChanged, win, updateMask);
#endif
#if defined(Q_OS_WIN)
        // Windows：延后 show，让 QML 完成布局和首帧后再显示，避免白屏；并启用 Win11 原生圆角
        QTimer::singleShot(80, win, [win, updateMask]() {
            win->show();
            win->raise();
            win->requestActivate();
            applyWindows11RoundedCorners(win);  // show 后设置 Win11 圆角
            updateMask();
        });
#else
        win->show();
        win->raise();
        win->requestActivate();
        updateMask();
#endif
    }

    // 组件注册改为由 QML 在加载对应组件页时调用 appController.registerPicMatchHost(hostItem)，
    // 不再在启动时查找 playerHost。

    // 关闭请求：用 QueuedConnection 在下一次事件循环执行，先关窗再延迟 quit，避免 macOS IMK/runloop 报错或无法退出
    QObject::connect(&appController, &AppController::requestQuit, &app, [&app, root]() {
        if (QQuickWindow *win = qobject_cast<QQuickWindow *>(root))
            win->close();
        QApplication::closeAllWindows();
        QTimer::singleShot(100, &app, &QApplication::quit);
    }, Qt::QueuedConnection);

    // 应用退出时调用各组件的 shutdown 符号（如 PicPlayer_Shutdown），释放组件资源
    QObject::connect(&app, &QApplication::aboutToQuit, &app, [shutdownList]() {
        for (const auto &e : shutdownList) {
            QLibrary lib(e.first);
            if (lib.load()) {
                typedef void (*ShutdownFunc)();
                auto fn = reinterpret_cast<ShutdownFunc>(lib.resolve(e.second.toUtf8().constData()));
                if (fn)
                    fn();
            }
        }
    });

    LOG_INFO("-------------------------------Application started (QML).");
    return app.exec();
}
