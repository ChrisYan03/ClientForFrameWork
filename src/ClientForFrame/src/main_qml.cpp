#include "QmlBridge/AppController.h"
#include "QmlBridge/PlayerHostItem.h"
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
#include "LogUtil.h"
#if defined(Q_OS_WIN)
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

    // 使用支持 background/contentItem 自定义的样式；Fusion 在 Windows 下比 Basic 更稳定，避免白屏
    QQuickStyle::setStyle("Fusion");

    // 确保日志目录存在（以 exe 所在目录为基准，便于 Release 下生成到 target/Win64/bin/Release/logs）
    QDir::setCurrent(QCoreApplication::applicationDirPath());
    QDir().mkpath("logs");

    LogUtil::initLogger("ClientApp");
    LOG_INFO("-------------------------------Application starting (QML)...");

    AppController appController;

    QQmlApplicationEngine engine;
    qmlRegisterType<PlayerHostItem>("App", 1, 0, "PlayerHostItem");
    engine.rootContext()->setContextProperty("appController", &appController);

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
        QObject::connect(win, &QWindow::visibilityChanged, win, updateMask);
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

    // 从 QML 场景中拿到嵌入的 PlayerHostItem，把 PicMatchWidget 交给 AppController（运行/停止用）
    // Windows 下窗口延后 show，此时 widget 可能尚未创建，需等 widgetReady 再 setPlayer
    QObject *hostObj = root->findChild<QObject *>("playerHost");
    if (PlayerHostItem *hostItem = qobject_cast<PlayerHostItem *>(hostObj)) {
        if (hostItem->picMatchWidget())
            appController.setPlayer(hostItem->picMatchWidget(), nullptr);
        QObject::connect(hostItem, &PlayerHostItem::widgetReady, &appController, [&appController, hostItem]() {
            if (hostItem->picMatchWidget()) {
                appController.setPlayer(hostItem->picMatchWidget(), nullptr);
                LOG_INFO("AppController setPlayer from widgetReady (PicMatchWidget now available)");
            }
        });
    }

    // 关闭请求：用 QueuedConnection 在下一次事件循环执行，先关窗再延迟 quit，避免 macOS IMK/runloop 报错或无法退出
    QObject::connect(&appController, &AppController::requestQuit, &app, [&app, root]() {
        if (QQuickWindow *win = qobject_cast<QQuickWindow *>(root))
            win->close();
        QApplication::closeAllWindows();
        QTimer::singleShot(100, &app, &QApplication::quit);
    }, Qt::QueuedConnection);

    LOG_INFO("-------------------------------Application started (QML).");
    return app.exec();
}
