/**
 * @file FrameworkComponentLoader.h
 * @brief 基于 Framework 的组件加载器
 *
 * 使用新的 Framework C 风格 API 进行组件加载和管理，
 * 完成主框架与组件的闭环连接。
 */
#ifndef FRAMEWORK_COMPONENT_LOADER_H
#define FRAMEWORK_COMPONENT_LOADER_H

#include "IComponentApi.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QQmlEngine>
#include <QPointer>

// 前向声明
class AppController;

/**
 * @brief 基于 Framework 的组件加载器
 *
 * 职责：
 * 1. 使用 Framework API 加载组件
 * 2. 管理组件生命周期
 * 3. 集成到主框架的 QML 系统
 */
class FrameworkComponentLoader : public QObject
{
    Q_OBJECT

public:
    explicit FrameworkComponentLoader(QObject *parent = nullptr);
    ~FrameworkComponentLoader() override;

    /**
     * @brief 加载所有启用的组件
     * @param engine QML引擎实例
     * @param appController 应用控制器
     * @return 是否成功
     */
    bool loadAllComponents(QQmlEngine *engine, QObject *appController);

    /**
     * @brief 加载单个组件
     * @param componentId 组件ID
     * @param basePath 组件基础路径
     * @param engine QML引擎实例
     * @return 是否成功
     */
    bool loadComponent(const QString &componentId, const QString &basePath, QQmlEngine *engine);

    /**
     * @brief 卸载所有组件
     */
    void unloadAllComponents();

    /**
     * @brief 获取组件句柄
     * @param componentId 组件ID
     * @return 组件句柄，不存在返回 nullptr
     */
    ComponentHandle getComponent(const QString &componentId) const;

    /**
     * @brief 获取组件元数据
     * @param componentId 组件ID
     * @param manifest 输出参数
     * @return 是否成功
     */
    bool getComponentManifest(const QString &componentId, ComponentManifest &manifest) const;

    /**
     * @brief 获取已加载的组件ID列表
     * @return 组件ID列表
     */
    QStringList getLoadedComponentIds() const;

signals:
    /**
     * @brief 组件加载完成信号
     * @param componentId 组件ID
     */
    void componentLoaded(const QString &componentId);

    /**
     * @brief 组件加载失败信号
     * @param componentId 组件ID
     * @param errorMessage 错误信息
     */
    void componentLoadFailed(const QString &componentId, const QString &errorMessage);

    /**
     * @brief 组件卸载完成信号
     * @param componentId 组件ID
     */
    void componentUnloaded(const QString &componentId);

private:
    /**
     * @brief 读取配置文件获取启用的组件列表
     * @return 组件ID列表
     */
    QStringList loadEnabledComponents();

    /**
     * @brief 注册组件到 QML 系统
     * @param componentId 组件ID
     * @param manifest 组件元数据
     * @param engine QML引擎实例
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
};

#endif // FRAMEWORK_COMPONENT_LOADER_H
