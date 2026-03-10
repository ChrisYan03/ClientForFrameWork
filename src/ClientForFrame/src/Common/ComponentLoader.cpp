#include "ComponentLoader.h"
#include "QmlBridge/AppController.h"
#include "LogUtil.h"
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLibrary>
#include <QQmlEngine>
#include <QObject>
#include <QUrl>
#if defined(Q_OS_WIN)
#include <Windows.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#ifndef DWMWA_WINDOW_CORNER_PREFERENCE
#define DWMWA_WINDOW_CORNER_PREFERENCE 33
#endif
#endif

ComponentLoader::ComponentLoader() = default;

QStringList ComponentLoader::loadEnabledComponents()
{
    QString baseDir = m_paths.initialBaseDir();
    auto tryOpen = [](const QString &p) -> QByteArray {
        QFile f(p);
        return f.open(QIODevice::ReadOnly | QIODevice::Text) ? f.readAll() : QByteArray();
    };
    QString path = QDir(baseDir).absoluteFilePath(QStringLiteral("config/components.json"));
    QByteArray jsonData = tryOpen(path);
    if (jsonData.isEmpty())
        path = QDir(baseDir).absoluteFilePath(QStringLiteral("config/component.json")), jsonData = tryOpen(path);
#if defined(Q_OS_MAC)
    if (jsonData.isEmpty()) {
        path = QDir(baseDir).absoluteFilePath(QStringLiteral("Release/config/components.json"));
        jsonData = tryOpen(path);
        if (jsonData.isEmpty()) {
            path = QDir(baseDir).absoluteFilePath(QStringLiteral("Release/config/component.json"));
            jsonData = tryOpen(path);
        }
        if (!jsonData.isEmpty())
            m_paths.setResolvedBaseDir(QDir(baseDir).absoluteFilePath(QStringLiteral("Release")));
    }
#endif
    if (jsonData.isEmpty()) {
        LOG_INFO("config not found (tried config/components.json, config/component.json, baseDir: {})", baseDir.toStdString());
        return QStringList();
    }
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        LOG_INFO("config parse error at {}", path.toStdString());
        return QStringList();
    }
    QJsonArray arr = doc.object().value(QStringLiteral("components")).toArray();
    QStringList list;
    for (const QJsonValue &v : arr) {
        QString id = v.toString().trimmed();
        if (!id.isEmpty())
            list.append(id);
    }
    baseDir = m_paths.baseDir();
    LOG_INFO("baseDir: {}, components: {} (from {})", baseDir.toStdString(), static_cast<int>(list.size()), path.toStdString());
    return list;
}

void ComponentLoader::loadComponentDll(QQmlEngine *engine, QObject *appController, const QString &componentId, QList<ShutdownEntry> *shutdownList)
{
    if (!engine || !appController)
        return;
    QString exeDir = m_paths.baseDir();
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
    QString appId = obj.value(QStringLiteral("id")).toString();
    QString iconFile = obj.value(QStringLiteral("icon")).toString();
    auto *controller = qobject_cast<AppController *>(appController);
    if (controller && !appId.isEmpty() && !iconFile.isEmpty()) {
        QString iconPath = exeDir + QStringLiteral("/Component/") + componentId + QStringLiteral("/meta_info/") + iconFile;
        controller->registerComponentIcon(appId, iconPath);
    }
    if (shutdownList && !shutdownModule.isEmpty() && !shutdownSymbol.isEmpty()) {
        QString shutdownPath;
#if defined(Q_OS_MAC)
        QString shutBase = shutdownModule;
        if (shutBase.endsWith(QStringLiteral(".dll")))
            shutBase = shutBase.left(shutBase.size() - 4);
        else if (shutBase.endsWith(QStringLiteral(".so")) || shutBase.endsWith(QStringLiteral(".dylib")))
            shutBase = shutBase.left(shutBase.lastIndexOf(QLatin1Char('.')));
        shutdownPath = exeDir + QStringLiteral("/Component/") + componentId + QStringLiteral("/bin/lib") + shutBase + QStringLiteral(".dylib");
#elif defined(Q_OS_WIN)
        shutdownPath = exeDir + QStringLiteral("/Component/") + componentId + QStringLiteral("/bin/") + shutdownModule;
#else
        QString shutBase = shutdownModule;
        if (shutBase.endsWith(QStringLiteral(".dll")))
            shutBase = shutBase.left(shutBase.size() - 4);
        else if (shutBase.endsWith(QStringLiteral(".so")) || shutBase.endsWith(QStringLiteral(".dylib")))
            shutBase = shutBase.left(shutBase.lastIndexOf(QLatin1Char('.')));
        shutdownPath = exeDir + QStringLiteral("/Component/") + componentId + QStringLiteral("/bin/lib") + shutBase + QStringLiteral(".so");
#endif
        shutdownList->append(qMakePair(shutdownPath, shutdownSymbol));
    }
    QString componentBin = exeDir + QStringLiteral("/Component/") + componentId + QStringLiteral("/bin");
    QString libPath;
#if defined(Q_OS_WIN)
    libPath = componentBin + QLatin1Char('/') + module;
    const DWORD SEARCH_DEFAULT = 0x00001000;
    const DWORD SEARCH_USER = 0x00000200;
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
#else
    QString baseName = module;
    if (baseName.endsWith(QStringLiteral(".dll")))
        baseName = baseName.left(baseName.size() - 4);
    else if (baseName.endsWith(QStringLiteral(".so")) || baseName.endsWith(QStringLiteral(".dylib")))
        baseName = baseName.left(baseName.lastIndexOf(QLatin1Char('.')));
#if defined(Q_OS_MAC)
    libPath = componentBin + QLatin1Char('/') + QStringLiteral("lib") + baseName + QStringLiteral(".dylib");
#else
    libPath = componentBin + QLatin1Char('/') + QStringLiteral("lib") + baseName + QStringLiteral(".so");
#endif
#endif
    QLibrary lib(libPath);
    if (!lib.load()) {
        LOG_INFO("Component {}: failed to load {}: {}", componentId.toStdString(), libPath.toStdString(), lib.errorString().toStdString());
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

    QString qmlPage = obj.value(QStringLiteral("qmlPage")).toString();
    if (controller && !appId.isEmpty() && !qmlPage.isEmpty()) {
        QString componentBin = exeDir + QStringLiteral("/Component/") + componentId + QStringLiteral("/bin");
        engine->addImportPath(componentBin + QStringLiteral("/qml"));
        controller->registerComponentPage(appId, QUrl::fromLocalFile(componentBin + QStringLiteral("/qml/") + qmlPage));
    }

    LOG_INFO("Component {}: registered", componentId.toStdString());
}

QList<ShutdownEntry> ComponentLoader::loadAllComponents(QQmlEngine *engine, QObject *appController)
{
    QList<ShutdownEntry> shutdownList;
    for (const QString &compId : loadEnabledComponents())
        loadComponentDll(engine, appController, compId, &shutdownList);
    return shutdownList;
}

void ComponentLoader::runShutdownList(const QList<ShutdownEntry> &shutdownList)
{
    for (const auto &e : shutdownList) {
        QLibrary lib(e.first);
        if (lib.load()) {
            typedef void (*ShutdownFunc)();
            auto fn = reinterpret_cast<ShutdownFunc>(lib.resolve(e.second.toUtf8().constData()));
            if (fn)
                fn();
        }
    }
}
