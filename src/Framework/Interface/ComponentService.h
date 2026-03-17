/**
 * @file ComponentService.h
 * @brief Framework 组件服务统一入口
 *
 * 提供简洁的 API 供主框架加载和管理组件。
 * 整合了 ComponentManager、组件生命周期管理、QML 注册等功能。
 */
#ifndef COMPONENT_SERVICE_H
#define COMPONENT_SERVICE_H

#include "IComponentData.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QQmlEngine>
#include <QVariantMap>

/**
 * @brief Framework 组件服务
 *
 * 统一入口类，整合以下功能：
 * - 读取组件配置 (components.json)
 * - 动态加载组件 (QLibrary + createComponent)
 * - 组件生命周期管理 (初始化、运行、关闭)
 * - 注册到 QML 系统
 * - 组件间通信协调
 *
 * 使用示例：
 * @code
 * // 主框架中
 * ComponentService service;
 * service.initialize(&engine, &appController);
 *
 * // 应用退出时
 * service.shutdown();
 * @endcode
 */
class ComponentService : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit ComponentService(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~ComponentService() override;

    // ==================== 初始化与关闭 ====================

    /**
     * @brief 初始化组件服务
     * @param engine QML 引擎指针
     * @param appController 应用控制器指针
     * @return 是否成功
     *
     * 执行以下操作：
     * 1. 创建组件管理器
     * 2. 读取 components.json 获取启用的组件列表
     * 3. 加载并初始化所有组件
     * 4. 注册组件到 QML 系统
     */
    bool initialize(QQmlEngine *engine, QObject *appController);

    /**
     * @brief 关闭组件服务
     *
     * 执行以下操作：
     * 1. 关闭所有组件
     * 2. 卸载所有组件
     * 3. 销毁组件管理器
     */
    void shutdown();

    // ==================== 组件管理 ====================

    /**
     * @brief 加载单个组件
     * @param componentId 组件 ID
     * @param basePath 组件基础路径
     * @return 是否成功
     */
    bool loadComponent(const QString &componentId, const QString &basePath);

    /**
     * @brief 卸载单个组件
     * @param componentId 组件 ID
     * @return 是否成功
     */
    bool unloadComponent(const QString &componentId);

    /**
     * @brief 获取组件句柄
     * @param componentId 组件 ID
     * @return 组件句柄，不存在返回 nullptr
     */
    ComponentHandle getComponent(const QString &componentId) const;

    /**
     * @brief 获取组件元数据
     * @param componentId 组件 ID
     * @param manifest 输出参数
     * @return 是否成功
     */
    bool getComponentManifest(const QString &componentId, ComponentManifest &manifest) const;

    /**
     * @brief 获取已加载的组件 ID 列表
     * @return 组件 ID 列表
     */
    QStringList getLoadedComponentIds() const;

    /**
     * @brief 检查组件是否已加载
     * @param componentId 组件 ID
     * @return 是否已加载
     */
    bool isComponentLoaded(const QString &componentId) const;

    // ==================== 配置 ====================

    /**
     * @brief 设置组件基础路径
     * @param basePath 基础路径
     */
    void setBasePath(const QString &basePath);

    /**
     * @brief 获取组件基础路径
     * @return 基础路径
     */
    QString basePath() const;

signals:
    /**
     * @brief 组件加载完成信号
     * @param componentId 组件 ID
     */
    void componentLoaded(const QString &componentId);

    /**
     * @brief 组件加载失败信号
     * @param componentId 组件 ID
     * @param errorMessage 错误信息
     */
    void componentLoadFailed(const QString &componentId, const QString &errorMessage);

    /**
     * @brief 组件卸载完成信号
     * @param componentId 组件 ID
     */
    void componentUnloaded(const QString &componentId);

    /**
     * @brief 所有组件加载完成信号
     * @param success 是否全部成功
     */
    void allComponentsLoaded(bool success);

private:
    /**
     * @brief 读取配置获取启用的组件列表
     * @return 组件 ID 列表
     */
    QStringList loadEnabledComponents();

    /**
     * @brief 注册组件到 QML 系统
     * @param componentId 组件 ID
     * @param manifest 组件元数据
     * @param engine QML 引擎
     * @param appController 应用控制器
     * @return 是否成功
     */
    bool registerComponentToQml(const QString &componentId,
                               const ComponentManifest &manifest,
                               QQmlEngine *engine,
                               QObject *appController);

private:
    ComponentManagerHandle m_manager = nullptr;
    QMap<QString, ComponentHandle> m_components;
    QString m_basePath;
    QQmlEngine *m_engine = nullptr;
    QObject *m_appController = nullptr;
    bool m_initialized = false;
};

#endif // COMPONENT_SERVICE_H
