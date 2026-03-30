/**
 * @file IComponentLoader.h
 * @brief 组件加载器接口定义
 *
 * 定义了组件加载器的抽象接口，支持4种类型的组件加载
 */
#ifndef ICOMPONENT_LOADER_H
#define ICOMPONENT_LOADER_H

#include "../include/main/ComponentTypes.h"
#include <QObject>
#include <QString>

/**
 * @brief 组件加载器接口
 *
 * 所有组件加载器必须实现此接口，提供统一的组件加载方式。
 */
class IComponentLoader
{
public:
    virtual ~IComponentLoader() = default;

    /**
     * @brief 加载组件
     * @param manifest 组件元数据
     * @param basePath 组件基础路径
     * @return 组件对象指针，失败返回 nullptr
     */
    virtual QObject* load(const ComponentManifest& manifest, const QString& basePath) = 0;

    /**
     * @brief 卸载组件
     * @param component 组件对象指针
     * @return 是否成功
     */
    virtual bool unload(QObject* component) = 0;

    /**
     * @brief 检查组件是否可加载
     * @param manifest 组件元数据
     * @param basePath 组件基础路径
     * @return 是否可加载
     */
    virtual bool canLoad(const ComponentManifest& manifest, const QString& basePath) const = 0;

    /**
     * @brief 获取加载器支持的组件类型
     * @return 组件类型枚举
     */
    virtual ComponentType supportedType() const = 0;
};

#endif // ICOMPONENT_LOADER_H