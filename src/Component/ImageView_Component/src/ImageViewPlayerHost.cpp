#include "PicPlayerApi.h"
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QTimer>
#include "LogUtil.h"

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOption({{"p", "parent-hwnd"}, "Host window handle", "parent-hwnd"});
    parser.addOption({{"w", "width"}, "Initial width", "width", "800"});
    parser.addOption({{"H", "height"}, "Initial height", "height", "600"});
    parser.process(app);

    LogUtil::initLogger("ImageViewPlayerHost");
    if (!PicPlayer_Init()) {
        LOG_ERROR("ImageViewPlayerHost: PicPlayer_Init failed");
        return -1;
    }

    const int handle = PicPlayer_CreateInstance();
    if (handle <= 0) {
        LOG_ERROR("ImageViewPlayerHost: PicPlayer_CreateInstance failed");
        PicPlayer_UnInit();
        return -1;
    }

#if defined(Q_OS_WIN)
    bool ok = false;
    const quintptr hwndVal = parser.value("parent-hwnd").toULongLong(&ok, 10);
    if (!ok || hwndVal == 0) {
        LOG_ERROR("ImageViewPlayerHost: invalid --parent-hwnd");
        PicPlayer_DestroyInstance(handle);
        PicPlayer_UnInit();
        return -1;
    }
    PicPlayer_RegisterWindow(handle, static_cast<Window_ShowID>(hwndVal));
    PicPlayer_SetWindowSize(handle, parser.value("width").toInt(), parser.value("height").toInt());
#endif

    PicPlayer_Play(handle);

    QObject::connect(&app, &QCoreApplication::aboutToQuit, [&]() {
        PicPlayer_DestroyInstance(handle);
        PicPlayer_UnInit();
    });

    QTimer idleTimer;
    idleTimer.setInterval(1000);
    QObject::connect(&idleTimer, &QTimer::timeout, []() {});
    idleTimer.start();
    return app.exec();
}
