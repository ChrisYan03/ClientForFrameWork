/**
 * @file ComponentManager.h
 * @brief 组件管理器类定义
 *
 * 此文件是 Framework 内部实现，对外部组件不可见。
 * 负责组件的生命周期管理、元数据解析、配置管理等。
 */
#ifndef COMPONENT_MANAGER_H
#define COMPONENT_MANAGER_H

#include "../Interface/IComponentData.h"
#include "../Interface/IComponent.h"
#include <QObject>
#include <QString>
#include <QMap>
#include <QSharedPointer>
#include <QQmlEngine>
#include <QLibrary>

/**
 * @brief 组件实例包装类
 *
 * 内部使用，封装组件的实际实现细节。
 */
class ComponentInstance : public QObject
{
    Q_OBJECT

public:
    explicit ComponentInstance(const QString &componentId, QObject *parent = nullptr);
    ~ComponentInstance() override;

    // 组件元数据（从 manifest.json 解析）
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

    // 组件基础路径
    QString basePath;

    // QML引擎
    QQmlEngine* qmlEngine = nullptr;

    // 暴露的接口对象
    QMap<QString, QObject*> interfaces;

    // 动态库
    QLibrary* library = nullptr;

    // 组件对象实例（通过 createComponent 创建）
    QObject* componentObject = nullptr;

    // IComponent 接口指针（用于调用组件的初始化等方法）
    IComponent* iComponent = nullptr;

    // 获取各种元数据信息的便捷方法
    QString getId() const { return QString::fromStdString(manifest.id); }
    QString getName() const { return QString::fromStdString(manifest.name); }
    QString getVersion() const { return QString::fromStdString(manifest.version); }
    QString getDescription() const { return QString::fromStdString(manifest.description); }
    QString getAuthor() const { return QString::fromStdString(manifest.author); }
    QString getIconPath() const { return QString::fromStdString(manifest.icon); }
    QString getQmlPage() const { return QString::fromStdString(manifest.qmlPage); }
    QString getDataPath() const { return QString::fromStdString(manifest.dataPath); }

    bool isInitialized() const { return state >= Running; }
};

/**
 * @brief 组件管理器类
 *
 * 负责：
 * 1. 组件的加载、卸载管理
 * 2. 组件元数据解析（使用 xpack）
 * 3. 组件配置管理
 * 4. 组件间通信协调
 */
class ComponentManager : public QObject
{
    Q_OBJECT

public:
    explicit ComponentManager(QObject *parent = nullptr);
    ~ComponentManager() override;

    /**
     * @brief 加载组件
     * @param componentId 组件ID
     * @param basePath 组件基础路径
     * @return 组件实例指针，失败返回 nullptr
     */
    ComponentInstance* loadComponent(const QString &componentId, const QString &basePath);

    /**
     * @brief 卸载组件
     * @param componentId 组件ID
     * @return 是否成功
     */
    bool unloadComponent(const QString &componentId);

    /**
     * @brief 获取组件实例
     * @param componentId 组件ID
     * @return 组件实例指针，不存在返回 nullptr
     */
    ComponentInstance* getComponent(const QString &componentId) const;

    /**
     * @brief 获取所有已加载的组件ID列表
     * @return 组件ID列表
     */
    QStringList getLoadedComponentIds() const;

private:
    QMap<QString, QSharedPointer<ComponentInstance>> m_components;
};

#endif // COMPONENT_MANAGER_H
