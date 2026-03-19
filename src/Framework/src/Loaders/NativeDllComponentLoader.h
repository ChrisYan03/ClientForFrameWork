/**
 * @file NativeDllComponentLoader.h
 * @brief 原生动态库组件加载器
 *
 * 支持加载 .dll (Windows) 和 .dylib (macOS) 组件
 */
#ifndef NATIVE_DLL_COMPONENT_LOADER_H
#define NATIVE_DLL_COMPONENT_LOADER_H

#include "../IComponentLoader.h"
#include "../interface/IComponent.h"
#include <QLibrary>
#include <QMap>

/**
 * @brief 原生动态库组件加载器
 *
 * 负责加载传统的动态库组件，通过 QLibrary 加载 .dll/.dylib 文件，
 * 并调用组件导出的 createComponent() 工厂函数创建组件实例。
 */
class NativeDllComponentLoader : public QObject, public IComponentLoader
{
    Q_OBJECT

public:
    explicit NativeDllComponentLoader(QObject* parent = nullptr);
    ~NativeDllComponentLoader() override;

    // IComponentLoader 接口实现
    QObject* load(const ComponentManifest& manifest, const QString& basePath) override;
    bool unload(QObject* component) override;
    bool canLoad(const ComponentManifest& manifest, const QString& basePath) const override;
    ComponentType supportedType() const override { return ComponentType_NativeDll; }

private:
    /**
     * @brief 解析动态库文件路径
     * @param manifest 组件元数据
     * @param basePath 基础路径
     * @return 动态库完整路径
     */
    QString resolveLibraryPath(const ComponentManifest& manifest, const QString& basePath) const;

    /**
     * @brief 创建组件实例
     * @param library 已加载的动态库
     * @return 组件对象指针
     */
    QObject* createComponentInstance(QLibrary* library);

    // 管理已加载的动态库，key为组件ID
    QMap<QString, QLibrary*> m_loadedLibraries;
};

#endif // NATIVE_DLL_COMPONENT_LOADER_H