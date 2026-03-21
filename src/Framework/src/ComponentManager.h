/**
 * @file ComponentManager.h
 * @brief 增强版组件管理器
 *
 * 支持4种组件类型的统一管理，使用加载器工厂模式
 */
#ifndef COMPONENT_MANAGER_H
#define COMPONENT_MANAGER_H

#include "../include/ComponentTypes.h"
#include "../Interface/IComponent.h"
#include "IComponentLoader.h"
#include "ComponentLoaderFactory.h"
#include <QObject>
#include <QString>
#include <QMap>
#include <QSharedPointer>
#include <QQmlEngine>

/**
 * @brief 增强版组件实例包装类
 *
 * 支持多种组件类型的统一管理
 */
class ComponentInstanceV2 : public QObject
{
    Q_OBJECT

public:
    explicit ComponentInstanceV2(const QString& componentId, QObject* parent = nullptr);
    ~ComponentInstanceV2() override;

    // 组件元数据
    ComponentManifest manifest;

    // 组件配置
    QVariantMap config;

    // 组件状态
    enum State {
        Unloaded,
        Loading,
        Loaded,
        Initializing,
        Running,
        Error,
        Shutdown
    };
    State state = Unloaded;

    // 组件类型
    ComponentType componentType = ComponentType_NativeDll;

    // 组件基础路径
    QString basePath;

    // QML引擎
    QQmlEngine* qmlEngine = nullptr;

    // 暴露的接口对象
    QMap<QString, QObject*> interfaces;

    // 组件对象实例（可能是IComponent、进程包装器等）
    QObject* componentObject = nullptr;

    // IComponent 接口指针（仅对NativeDll组件有效）
    IComponent* iComponent = nullptr;

    // 组件加载器（用于卸载）
    IComponentLoader* loader = nullptr;

    // 便捷方法
    QString getId() const { return QString::fromStdString(manifest.id); }
    QString getName() const { return QString::fromStdString(manifest.name); }
    QString getVersion() const { return QString::fromStdString(manifest.version); }
    ComponentType getType() const { return componentType; }

    bool isInitialized() const { return state >= Running; }
};

/**
 * @brief 增强版组件管理器
 *
 * 支持4种组件类型的统一管理：
 * - NativeDll: 原生动态库
 * - StandaloneExe: 独立可执行文件
 * - WebUrl: Web URL组件
 * - EmbeddedExe: 嵌入式可执行文件
 */
class ComponentManagerV2 : public QObject
{
    Q_OBJECT

public:
    explicit ComponentManagerV2(QObject* parent = nullptr);
    ~ComponentManagerV2() override;

    /**
     * @brief 加载组件（自动识别类型）
     * @param componentId 组件ID
     * @param basePath 组件基础路径
     * @return 组件实例指针，失败返回 nullptr
     */
    ComponentInstanceV2* loadComponent(const QString& componentId, const QString& basePath);

    /**
     * @brief 卸载组件
     * @param componentId 组件ID
     * @return 是否成功
     */
    bool unloadComponent(const QString& componentId);

    /**
     * @brief 获取组件实例
     * @param componentId 组件ID
     * @return 组件实例指针，不存在返回 nullptr
     */
    ComponentInstanceV2* getComponent(const QString& componentId) const;

    /**
     * @brief 获取所有已加载的组件ID列表
     * @return 组件ID列表
     */
    QStringList getLoadedComponentIds() const;

    /**
     * @brief 根据类型获取组件列表
     * @param type 组件类型
     * @return 组件ID列表
     */
    QStringList getComponentsByType(ComponentType type) const;

    /**
     * @brief 检查组件类型是否支持
     * @param type 组件类型
     * @return 是否支持
     */
    bool isComponentTypeSupported(ComponentType type) const;

signals:
    /**
     * @brief 组件加载成功信号
     * @param componentId 组件ID
     */
    void componentLoaded(const QString& componentId);

    /**
     * @brief 组件加载失败信号
     * @param componentId 组件ID
     * @param error 错误信息
     */
    void componentLoadFailed(const QString& componentId, const QString& error);

    /**
     * @brief 组件卸载信号
     * @param componentId 组件ID
     */
    void componentUnloaded(const QString& componentId);

private:
    /**
     * @brief 从manifest中解析组件类型
     * @param manifest 组件元数据
     * @return 组件类型枚举
     */
    ComponentType parseComponentType(const ComponentManifest& manifest) const;

    /**
     * @brief 字符串类型转换为枚举
     * @param typeStr 类型字符串
     * @return 组件类型枚举
     */
    ComponentType stringToComponentType(const std::string& typeStr) const;

    // 组件实例存储
    QMap<QString, QSharedPointer<ComponentInstanceV2>> m_components;

    // 加载器工厂
    ComponentLoaderFactory* m_loaderFactory;
};

#endif // COMPONENT_MANAGER_V2_H