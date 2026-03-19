/**
 * @file ComponentService.h
 * @brief 主框架组件服务接口
 *
 * 这是主框架加载组件时使用的唯一头文件
 * 包含了组件服务、类型定义和配置相关的所有接口
 *
 * 使用示例：
 * #include <Framework/ComponentService.h>
 *
 * ComponentService service;
 * service.setBasePath("/path/to/components");
 * service.initialize(&engine, &controller);
 */
#ifndef FRAMEWORK_COMPONENT_SERVICE_H
#define FRAMEWORK_COMPONENT_SERVICE_H

#include "ComponentTypes.h"
#include <QObject>
#include <QString>
#include <QQmlEngine>

// 前向声明，避免暴露内部实现
class ComponentManagerPrivate;

/**
 * @brief 组件服务类
 *
 * 这是主框架与组件系统交互的主要接口。
 * 主框架只需要包含这个头文件即可使用完整的组件功能。
 */
class ComponentService : public QObject
{
    Q_OBJECT

public:
    explicit ComponentService(QObject* parent = nullptr);
    ~ComponentService();

    /**
     * @brief 设置组件基础路径
     * @param basePath 组件根目录
     */
    void setBasePath(const QString& basePath);

    /**
     * @brief 初始化组件服务
     * @param engine QML引擎
     * @param appController 应用控制器
     * @return 是否成功
     */
    bool initialize(QQmlEngine* engine, QObject* appController);

    /**
     * @brief 关闭组件服务
     */
    void shutdown();

    /**
     * @brief 加载指定组件
     * @param componentId 组件ID
     * @return 是否成功
     */
    bool loadComponent(const QString& componentId);

    /**
     * @brief 卸载指定组件
     * @param componentId 组件ID
     * @return 是否成功
     */
    bool unloadComponent(const QString& componentId);

    /**
     * @brief 获取已加载组件列表
     * @return 组件ID列表
     */
    QStringList loadedComponents() const;

    /**
     * @brief 根据类型获取组件列表
     * @param type 组件类型
     * @return 组件ID列表
     */
    QStringList getComponentsByType(ComponentType type) const;

signals:
    void componentLoaded(const QString& componentId);
    void componentLoadFailed(const QString& componentId, const QString& error);
    void componentUnloaded(const QString& componentId);

private:
    // 前向声明
    class ComponentManagerPrivate;
    ComponentManagerPrivate* d;
};

#endif // FRAMEWORK_COMPONENT_SERVICE_H