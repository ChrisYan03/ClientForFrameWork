/**
 * @file ComponentLoaderFactory.h
 * @brief 组件加载器工厂
 *
 * 根据组件类型创建对应的组件加载器实例
 */
#ifndef COMPONENT_LOADER_FACTORY_H
#define COMPONENT_LOADER_FACTORY_H

#include "IComponentLoader.h"
#include "../include/main/ComponentTypes.h"
#include <QMap>
#include <QObject>

/**
 * @brief 组件加载器工厂类
 *
 * 负责根据组件类型创建相应���加载器实例。
 * 采用工厂模式，支持扩展新的组件类型。
 */
class ComponentLoaderFactory : public QObject
{
    Q_OBJECT

public:
    explicit ComponentLoaderFactory(QObject* parent = nullptr);
    ~ComponentLoaderFactory() override;

    /**
     * @brief 获取指定类型的加载器
     * @param type 组件类型枚举
     * @return 组件加载器指针，如果类型不支持返回 nullptr
     */
    IComponentLoader* getLoader(ComponentType type);

    /**
     * @brief 注册自定义加载器
     * @param type 组件类型
     * @param loader 加载器实例（工厂会接管所有权）
     * @return 是否注册成功
     */
    bool registerLoader(ComponentType type, IComponentLoader* loader);

    /**
     * @brief 检查是否支持指定类型
     * @param type 组件类型
     * @return 是否支持
     */
    bool isTypeSupported(ComponentType type) const;

    /**
     * @brief 获取所有支持的组件类型
     * @return 支持的组件类型列表
     */
    QList<ComponentType> supportedTypes() const;

private:
    /**
     * @brief 初始化内置加载器
     */
    void initializeBuiltInLoaders();

    // 存储各种类型的加载器
    QMap<ComponentType, IComponentLoader*> m_loaders;
};

#endif // COMPONENT_LOADER_FACTORY_H