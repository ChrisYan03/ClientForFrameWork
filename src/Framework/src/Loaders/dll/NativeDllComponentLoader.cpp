/**
 * @file NativeDllComponentLoader.cpp
 * @brief 原生动态库组件加载器实现
 */
#include "NativeDllComponentLoader.h"
#include "../../../Interface/IComponent.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include "LogUtil.h"

NativeDllComponentLoader::NativeDllComponentLoader(QObject* parent)
    : QObject(parent)
{
}

NativeDllComponentLoader::~NativeDllComponentLoader()
{
    for (auto it = m_loadedLibraries.begin(); it != m_loadedLibraries.end(); ++it) {
        QLibrary* library = it.value();
        if (library) {
            if (library->isLoaded()) {
                library->unload();
            }
            delete library;
        }
    }
    m_loadedLibraries.clear();
}

QObject* NativeDllComponentLoader::load(const ComponentManifest& manifest, const QString& basePath)
{
    QString libPath = resolveLibraryPath(manifest, basePath);
    if (libPath.isEmpty()) {
        LOG_WARN("Failed to resolve library path for component: {}",
                QString::fromStdString(manifest.id).toStdString());
        return nullptr;
    }

#ifdef Q_OS_WIN
    QString componentBinPath = basePath + QStringLiteral("/bin");
    QByteArray originalPath = qgetenv("PATH");
    qputenv("PATH", (componentBinPath + ";" + QString::fromLocal8Bit(originalPath)).toUtf8());
#endif

    QLibrary* library = new QLibrary(libPath);
    if (!library->load()) {
        LOG_WARN("Failed to load library: {} error: {}",
                libPath.toStdString(), library->errorString().toStdString());

#ifdef Q_OS_WIN
        qputenv("PATH", originalPath);
#endif
        delete library;
        return nullptr;
    }

#ifdef Q_OS_WIN
    qputenv("PATH", originalPath);
#endif

    QObject* component = createComponentInstance(library);
    if (!component) {
        LOG_WARN("Failed to create component instance from: {}", libPath.toStdString());
        delete library;
        return nullptr;
    }

    QString componentId = QString::fromStdString(manifest.id);
    m_loadedLibraries[componentId] = library;

    return component;
}

bool NativeDllComponentLoader::unload(QObject* component)
{
    if (!component) {
        return false;
    }

    IComponent* iComponent = dynamic_cast<IComponent*>(component);
    if (iComponent) {
        iComponent->shutdown();
    }

    delete component;

    for (auto it = m_loadedLibraries.begin(); it != m_loadedLibraries.end(); ++it) {
        QLibrary* library = it.value();
        if (library) {
            if (library->isLoaded()) {
                library->unload();
            }
            delete library;
        }
    }
    m_loadedLibraries.clear();

    return true;
}

bool NativeDllComponentLoader::canLoad(const ComponentManifest& manifest, const QString& basePath) const
{
    QString libPath = resolveLibraryPath(manifest, basePath);
    LOG_INFO("NativeDllComponentLoader::canLoad checking: {}", libPath.toStdString());

    if (!QFile::exists(libPath)) {
        LOG_WARN("DLL file not found: {}", libPath.toStdString());
        return false;
    }

#ifdef Q_OS_WIN
    QString componentBinPath = basePath + QStringLiteral("/bin");
    QByteArray originalPath = qgetenv("PATH");
    qputenv("PATH", (componentBinPath + ";" + QString::fromLocal8Bit(originalPath)).toUtf8());
    LOG_INFO("Set PATH to include: {}", componentBinPath.toStdString());
#endif

    QLibrary library(libPath);
    LOG_INFO("Attempting to load: {}", libPath.toStdString());
    bool loaded = library.load();

#ifdef Q_OS_WIN
    qputenv("PATH", originalPath);
#endif

    if (!loaded) {
        LOG_WARN("Cannot load component (missing dependencies): {} error: {}",
                libPath.toStdString(), library.errorString().toStdString());
    } else {
        LOG_INFO("Successfully loaded: {}", libPath.toStdString());
        library.unload();
    }

    return loaded;
}

QString NativeDllComponentLoader::resolveLibraryPath(const ComponentManifest& manifest, const QString& basePath) const
{
    LOG_INFO("resolveLibraryPath: basePath = {}", basePath.toStdString());
    QString libPath = basePath;
    QString moduleName;
    if (!manifest.module.empty()) {
        moduleName = QString::fromStdString(manifest.module);
    } else {
        moduleName = QString::fromStdString(manifest.id) + "Component";
    }

    if (moduleName.contains('/') || moduleName.contains('\\')) {
        libPath = basePath + "/" + moduleName;
    } else {
        libPath = basePath + "/bin/" + moduleName;
    }

    bool hasSuffix = false;
#if defined(Q_OS_WIN)
    hasSuffix = moduleName.endsWith(QStringLiteral(".dll"), Qt::CaseInsensitive);
#elif defined(Q_OS_MAC)
    hasSuffix = moduleName.endsWith(QStringLiteral(".dylib"), Qt::CaseInsensitive);
#else
    hasSuffix = moduleName.endsWith(QStringLiteral(".so"), Qt::CaseInsensitive);
#endif

#if defined(Q_OS_WIN)
    if (!hasSuffix) {
        libPath += QStringLiteral(".dll");
    }
#elif defined(Q_OS_MAC)
    if (!hasSuffix) {
        const QFileInfo fi(libPath);
        libPath = fi.path() + QStringLiteral("/lib") + fi.completeBaseName() + QStringLiteral(".dylib");
    }
#else
    if (!hasSuffix) {
        const QFileInfo fi(libPath);
        libPath = fi.path() + QStringLiteral("/lib") + fi.completeBaseName() + QStringLiteral(".so");
    }
#endif

    LOG_INFO("resolveLibraryPath: resolved to = {}", libPath.toStdString());
    return libPath;
}

QObject* NativeDllComponentLoader::createComponentInstance(QLibrary* library)
{
    if (!library || !library->isLoaded()) {
        return nullptr;
    }
    typedef void* (*CreateComponentFunc)();
    auto createFn = reinterpret_cast<CreateComponentFunc>(
        library->resolve("createComponent")
    );
    if (!createFn) {
        LOG_WARN("Failed to resolve createComponent symbol");
        return nullptr;
    }
    void* componentVoid = createFn();
    if (!componentVoid) {
        LOG_WARN("createComponent returned null");
        return nullptr;
    }
    return static_cast<QObject*>(componentVoid);
}
