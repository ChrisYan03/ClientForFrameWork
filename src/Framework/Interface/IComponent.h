/**
 * @file IComponent.h
 * @brief Framework 组件接口定义
 *
 * 所有组件必须实现此接口，供 Framework 统一管理组件生命周期。
 * 组件信息通过 ComponentManifest 获取，此接口只定义核心生命周期方法。
 */
#ifndef ICOMPONENT_H
#define ICOMPONENT_H

#include <QObject>
#include <QString>
#include <QQmlEngine>

/**
 * @brief 组件接口（纯虚基类）
 *
 * 所有组件必须继承此类并实现核心虚函数。
 * 组件信息(id/name/version/icon/qmlPage等)通过 ComponentManifest 获取，
 * Framework 会自动从 manifest.json 解析并填充到组件实例中。
 *
 * 使用方式：
 * 1. 继承此类和 QObject
 * 2. 声明 Q_OBJECT
 * 3. 实现 initialize/shutdown/registerQmlTypes/getInterface 四个核心方法
 * 4. 导出 createComponent() 工厂函数
 */
class IComponent
{
public:
    /**
     * @brief 虚析构函数
     */
    virtual ~IComponent() = default;

    // ==================== 生命周期 ====================

    /**
     * @brief 初始化组件
     * @param engine QML引擎指针
     * @param basePath 组件基础路径
     * @return 是否成功
     *
     * 在组件被加载后立即调用，负责：
     * - 初始化内部状态
     * - 注册QML类型
     * - 创建ViewModel等
     */
    virtual bool initialize(QQmlEngine *engine, const QString &basePath) = 0;

    /**
     * @brief 关闭组件
     *
     * 在组件被卸载前调用，负责：
     * - 保存组件状态
     * - 释放资源
     * - 停止后台任务
     */
    virtual void shutdown() = 0;

    // ==================== QML 集成 ====================

    /**
     * @brief 注册 QML 类型
     * @param engine QML引擎指针
     *
     * 由 Framework 调用，注册组件的 QML 类型、模块、导入路径等。
     * 可在 initialize 中调用，也可以在此方法中单独处理。
     */
    virtual void registerQmlTypes(QQmlEngine *engine) = 0;

    // ==================== 组件间通信 ====================

    /**
     * @brief 获取组件暴露的接口
     * @param interfaceName 接口名称
     * @return 接口对象指针，不存在返回 nullptr
     *
     * 供其他组件或主框架获取本组件的功能接口。
     * 例如：获取 ViewModel、获取服务接口等。
     */
    virtual QObject* getInterface(const QString &interfaceName) = 0;
};

#endif // ICOMPONENT_H
