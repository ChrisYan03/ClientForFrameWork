/**
 * @file NativeDllComponentLoader.h
 * @brief 原生动态库组件加载器
 *
 * 支持加载 .dll (Windows) 和 .dylib (macOS) 组件
 */
#ifndef NATIVE_DLL_COMPONENT_LOADER_H
#define NATIVE_DLL_COMPONENT_LOADER_H

#include "../../IComponentLoader.h"
#include "../../../Interface/IComponent.h"
#include <QLibrary>
#include <QMap>

class NativeDllComponentLoader : public QObject, public IComponentLoader
{
    Q_OBJECT

public:
    explicit NativeDllComponentLoader(QObject* parent = nullptr);
    ~NativeDllComponentLoader() override;

    QObject* load(const ComponentManifest& manifest, const QString& basePath) override;
    bool unload(QObject* component) override;
    bool canLoad(const ComponentManifest& manifest, const QString& basePath) const override;
    ComponentType supportedType() const override { return ComponentType_NativeDll; }

private:
    QString resolveLibraryPath(const ComponentManifest& manifest, const QString& basePath) const;
    QObject* createComponentInstance(QLibrary* library);

    QMap<QString, QLibrary*> m_loadedLibraries;
};

#endif // NATIVE_DLL_COMPONENT_LOADER_H
