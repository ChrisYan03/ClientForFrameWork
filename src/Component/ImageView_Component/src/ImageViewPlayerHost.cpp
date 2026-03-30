#include "ImageViewPlayerHostPicLoader.h"
#include "PicPlayerApi.h"
#include "ipc/ComponentIpcClient.h"
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include "LogUtil.h"

namespace {

QString resolvePicDataDir()
{
    const QString primary = QStringLiteral("E:/ClientForFrameWork/imagedata");
    if (QDir(primary).exists())
        return QDir(primary).absolutePath();

    const QString nearExe = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(QStringLiteral("picdata"));
    if (QDir(nearExe).exists())
        return QDir(nearExe).absolutePath();

    const QString upTree = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(QStringLiteral("../../../../picdata"));
    if (QDir(upTree).exists())
        return QDir(upTree).absolutePath();

    return primary;
}

QStringList firstTwoImageFiles(const QString& dirPath)
{
    QDir d(dirPath);
    if (!d.exists())
        return {};
    QStringList filters;
    filters << QStringLiteral("*.jpg")
            << QStringLiteral("*.jpeg")
            << QStringLiteral("*.JPG")
            << QStringLiteral("*.JPEG")
            << QStringLiteral("*.png")
            << QStringLiteral("*.PNG");
    d.setNameFilters(filters);
    d.setFilter(QDir::Files | QDir::Readable);
    d.setSorting(QDir::Name);
    QStringList out;
    const QStringList names = d.entryList();
    for (const QString& name : names) {
        out.append(d.absoluteFilePath(name));
        if (out.size() >= 2)
            break;
    }
    return out;
}

void pushPicFolderToPlayer(int handle, const QString& folder)
{
    const QString absDir = QDir(folder).absolutePath();
    LOG_INFO("ImageViewPlayerHost: picdata directory = {}", absDir.toStdString());

    const QStringList paths = firstTwoImageFiles(absDir);
    if (paths.size() < 2) {
        LOG_ERROR("ImageViewPlayerHost: need at least 2 images (jpg/png) under {}, found {}",
            absDir.toStdString(),
            paths.size());
        return;
    }

    for (const QString& path : paths) {
        PicShowInfo info;
        QString err;
        if (!ImageViewLoadImageFileToPicShowInfo(path, info, &err)) {
            LOG_ERROR("ImageViewPlayerHost: load failed {} ({})", path.toStdString(), err.toStdString());
            continue;
        }
        if (!PicPlayer_InputPicData(handle, 1, &info)) {
            LOG_ERROR("ImageViewPlayerHost: PicPlayer_InputPicData failed for {}", path.toStdString());
        }
    }
}

} // namespace

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOption({{"p", "parent-hwnd"}, "Host window handle", "parent-hwnd"});
    parser.addOption({{"w", "width"}, "Initial width", "width", "800"});
    parser.addOption({{"H", "height"}, "Initial height", "height", "600"});
    parser.addOption({{"d", "picdata-dir"}, "Folder with JPG/PNG (default: E:/ClientForFrameWork/picdata or ./picdata)", "dir"});
    parser.addOption(QCommandLineOption(QStringList{QStringLiteral("ipc-endpoint")}, QStringLiteral("Framework IPC endpoint"), QStringLiteral("ipc-endpoint")));
    parser.addOption(QCommandLineOption(QStringList{QStringLiteral("ipc-token")}, QStringLiteral("Framework IPC token"), QStringLiteral("ipc-token")));
    parser.addOption(QCommandLineOption(QStringList{QStringLiteral("component-id")}, QStringLiteral("Component id"), QStringLiteral("component-id"), QStringLiteral("ImageView")));
    parser.process(app);

    LogUtil::initLogger("ImageViewPlayerHost");
    if (!PicPlayer_Init()) {
        LOG_ERROR("ImageViewPlayerHost: PicPlayer_Init failed");
        return -1;
    }

    // 静态双图会保留两张纹理，cacheNum 须 >= 2，否则 PicPlayerImageByScene 会按 cache 淘汰一张
    const int handle = PicPlayer_CreateInstance(2, PicShowType_Image);
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

    const QString picDir = parser.isSet(QStringLiteral("picdata-dir"))
        ? parser.value(QStringLiteral("picdata-dir"))
        : resolvePicDataDir();
    QTimer::singleShot(400, [handle, picDir]() { pushPicFolderToPlayer(handle, picDir); });

    ComponentIpcClient ipcClient;
    if (parser.isSet(QStringLiteral("ipc-endpoint")) && parser.isSet(QStringLiteral("ipc-token"))) {
        const QString endpoint = parser.value(QStringLiteral("ipc-endpoint"));
        const QString token = parser.value(QStringLiteral("ipc-token"));
        const QString componentId = parser.value(QStringLiteral("component-id"));
        if (ipcClient.connectToHost(endpoint, token, componentId, 3000)) {
            ipcClient.sendEvent(QStringLiteral("component.state.running"), QJsonObject{});
            QObject::connect(&ipcClient, &ComponentIpcClient::notificationReceived, &app,
                [&](const QString& method, const QJsonObject& payload) {
                    if (method == QStringLiteral("framework.sync.fullState")) {
                        const int language = payload.value(QStringLiteral("language")).toInt(-1);
                        const QJsonObject theme = payload.value(QStringLiteral("theme")).toObject();
                        LOG_INFO("ImageViewPlayerHost: recv fullState language={} themeKeys={}",
                            language,
                            theme.keys().size());
                    } else if (method == QStringLiteral("framework.sync.language")) {
                        const int language = payload.value(QStringLiteral("language")).toInt(-1);
                        LOG_INFO("ImageViewPlayerHost: recv language={}", language);
                    } else if (method == QStringLiteral("framework.sync.theme")) {
                        const QJsonObject theme = payload.value(QStringLiteral("theme")).toObject();
                        LOG_INFO("ImageViewPlayerHost: recv theme update keys={}", theme.keys().size());
                    } else if (method == QStringLiteral("framework.lifecycle.stop")) {
                        QCoreApplication::quit();
                    }
                });
        } else {
            LOG_WARN("ImageViewPlayerHost: IPC connect failed, continue without runtime bridge");
        }
    }

    QObject::connect(&app, &QCoreApplication::aboutToQuit, [&]() {
        if (ipcClient.isConnected())
            ipcClient.sendEvent(QStringLiteral("component.state.stopped"), QJsonObject{});
        PicPlayer_DestroyInstance(handle);
        PicPlayer_UnInit();
    });

    QTimer idleTimer;
    idleTimer.setInterval(1000);
    QObject::connect(&idleTimer, &QTimer::timeout, []() {});
    idleTimer.start();
    return app.exec();
}
