/**
 * @file NativeDllComponentLoader.cpp
 * @brief 原生动态库组件加载器实现
 */
#include "NativeDllComponentLoader.h"
#include "../interface/IComponent.h"
#include <QDir>
#include "LogUtil.h"

NativeDllComponentLoader::NativeDllComponentLoader(QObject* parent)
    : QObject(parent)
{
}

NativeDllComponentLoader::~NativeDllComponentLoader()
{
    // 清理所有已加载的动态库
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

    // 在Windows上设置PATH环境变量以解决DLL依赖问题
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

    // 存储动态库引用，以便后续卸载
    QString componentId = QString::fromStdString(manifest.id);
    m_loadedLibraries[componentId] = library;

    return component;
}

bool NativeDllComponentLoader::unload(QObject* component)
{
    if (!component) {
        return false;
    }

    // 尝试转换为IComponent接口
    IComponent* iComponent = dynamic_cast<IComponent*>(component);
    if (iComponent) {
        iComponent->shutdown();
    }

    // 查找并卸载对应的动态库
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

    delete component;
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

    // 在Windows上设置PATH环境变量以解决DLL依赖问题
    // 即使DLL文件存在，其依赖的其他DLL可能需要PATH才能找到
#ifdef Q_OS_WIN
    QString componentBinPath = basePath + QStringLiteral("/bin");
    QByteArray originalPath = qgetenv("PATH");
    qputenv("PATH", (componentBinPath + ";" + QString::fromLocal8Bit(originalPath)).toUtf8());
    LOG_INFO("Set PATH to include: {}", componentBinPath.toStdString());
#endif

    // 尝试加载DLL来验证所有依赖都满足
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

    // 从manifest中获取模块名，如果没有则使用默认规则
    QString moduleName;
    if (!manifest.module.empty()) {
        moduleName = QString::fromStdString(manifest.module);
    } else {
        moduleName = QString::fromStdString(manifest.id) + "Component";
    }

    // 如果module是完整路径（包含/或\），直接使用
    // 否则假定module是相对于bin目录的文件名
    if (moduleName.contains('/') || moduleName.contains('\\')) {
        libPath = basePath + "/" + moduleName;
    } else {
        libPath = basePath + "/bin/" + moduleName;
    }

    // 如果moduleName已经包含.dll/.so/.dylib后缀，不再添加
    bool hasSuffix = false;
#if defined(Q_OS_WIN)
    hasSuffix = moduleName.endsWith(".dll", Qt::CaseInsensitive);
#elif defined(Q_OS_MAC)
    hasSuffix = moduleName.endsWith(".dylib", Qt::CaseInsensitive);
#else
    hasSuffix = moduleName.endsWith(".so", Qt::CaseInsensitive);
#endif

#if defined(Q_OS_WIN)
    if (!hasSuffix) libPath += ".dll";
#elif defined(Q_OS_MAC)
    if (!hasSuffix) libPath = libPath + "lib" + ".dylib";
#else
    if (!hasSuffix) libPath = libPath + "lib" + ".so";
#endif

    LOG_INFO("resolveLibraryPath: resolved to = {}", libPath.toStdString());
    return libPath;
}

QObject* NativeDllComponentLoader::createComponentInstance(QLibrary* library)
{
    if (!library || !library->isLoaded()) {
        return nullptr;
    }

    // 解析 createComponent 导出函数
    typedef void* (*CreateComponentFunc)();
    auto createFn = reinterpret_cast<CreateComponentFunc>(
        library->resolve("createComponent")
    );

    if (!createFn) {
        LOG_WARN("Failed to resolve createComponent symbol");
        return nullptr;
    }

    // 调用工厂函数创建组件实例
    void* componentVoid = createFn();
    if (!componentVoid) {
        LOG_WARN("createComponent returned null");
        return nullptr;
    }

    return static_cast<QObject*>(componentVoid);
}
