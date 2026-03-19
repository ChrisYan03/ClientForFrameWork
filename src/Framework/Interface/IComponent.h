/**
 * @file IComponent.h
 * @brief 组件接口定义
 *
 * 这是组件实现时需要包含的头文件
 * 不依赖QObject，避免多重继承问题
 *
 * 使用示例：
 * #include <Framework/IComponent.h>
 *
 * class MyComponent : public IComponent {
 *     int initialize(void* engine, const char* basePath) override;
 *     void shutdown() override;
 *     void registerQmlTypes(void* engine) override;
 *     void* getInterface(const char* name) override;
 * };
 */
#ifndef FRAMEWORK_ICOMPONENT_H
#define FRAMEWORK_ICOMPONENT_H

// 不依赖Qt头文件，使用C++标准类型
#include <cstdint>

// 包含组件类型定义（ComponentManifest等）
#include "../include/ComponentTypes.h"

/**
 * @brief 组件接口
 *
 * 纯虚接口，不依赖QObject，组件只需要实现此接口即可。
 * 避免了多重继承的问题，组件可以自由选择自己的基类。
 *
 * 接口设计原则：
 * - 使用void*避免Qt依赖
 * - 使用const char*避免QString依赖
 * - 返回int而非bool，跨语言兼容
 */
class IComponent
{
public:
    virtual ~IComponent() = default;

    /**
     * @brief 初始化组件
     * @param engine QML引擎指针 (void*类型，使用时需转换为QQmlEngine*)
     * @param basePath 组件基础路径 (UTF-8编码的C字符串)
     * @return 是否成功 (1=成功, 0=失败)
     *
     * 使用示例：
     * QQmlEngine* qmlEngine = static_cast<QQmlEngine*>(engine);
     * QString path = QString::fromUtf8(basePath);
     */
    virtual int initialize(void* engine, const char* basePath) = 0;

    /**
     * @brief 关闭组件
     *
     * 组件被卸载前调用，用于清理资源
     */
    virtual void shutdown() = 0;

    /**
     * @brief 注册QML类型
     * @param engine QML引擎指针
     *
     * 用于注册组件的QML类型到框架
     */
    virtual void registerQmlTypes(void* engine) = 0;

    /**
     * @brief 获取组件暴露的接口
     * @param interfaceName 接口名称 (UTF-8编码的C字符串)
     * @return 接口对象指针，不存在返回nullptr
     *
     * 用于组件间通信，返回组件提供的特定接口
     */
    virtual void* getInterface(const char* interfaceName) = 0;

    /**
     * @brief 设置组件元数据
     * @param manifest 组件元数据指针 (ComponentManifest*)
     *
     * Framework会在加载组件后调用此方法传递元数据
     * 组件可以选择性重写此方法来获取配置信息
     *
     * 默认实现为空，组件如果不关心元数据可以不重写
     */
    virtual void setManifest(const void* manifest) {
        // 默认空实现，避免强制组件处理元数据
        (void)manifest;
    }
};

// ==================== 组件导出宏定义 ====================

/**
 * @def COMPONENT_EXPORT
 * @brief 组件导出宏
 *
 * 跨平台的组件导出声明
 */
#ifdef _WIN32
    #define COMPONENT_EXPORT __declspec(dllexport)
#else
    #define COMPONENT_EXPORT __attribute__((visibility("default")))
#endif

/**
 * @def COMPONENT_CREATE_FUNC
 * @brief 组件工厂函数名称
 *
 * 所有组件DLL必须导出此名称的工厂函数
 */
#define COMPONENT_CREATE_FUNC createComponent

// ==================== 导出函数类型定义 ====================

extern "C" {
    /**
     * @brief 组件工厂函数类型
     *
     * 所有组件DLL必须实现此签名的函数
     * 返回新创建的IComponent接口指针
     */
    typedef IComponent* (*ComponentCreateFunc)();
}

#endif // FRAMEWORK_ICOMPONENT_H