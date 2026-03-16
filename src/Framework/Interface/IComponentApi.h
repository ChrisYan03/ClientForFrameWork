/**
 * @file IComponentApi.h
 * @brief Framework 组件 C 风格 API 接口
 *
 * 此文件提供纯 C 风格的函数接口，不暴露任何类定义。
 * 所有数据结构定义在 IComponentData.h 中，此文件只包含函数接口。
 *
 * 设计原则：
 * - 纯 C 风格函数接口
 * - 不暴露任何类定义
 * - 句柄（Handle）机制管理对象
 * - 接口稳定，向后兼容
 * - 数据结构完全分离
 */
#ifndef ICOMPONENT_API_H
#define ICOMPONENT_API_H

// 引用数据结构定义
#include "IComponentData.h"

// ==================== 版本信息 ====================

#define ICOMPONENT_API_VERSION_MAJOR 1
#define ICOMPONENT_API_VERSION_MINOR 0
#define ICOMPONENT_API_VERSION_PATCH 0

#define ICOMPONENT_API_VERSION_STRING "1.0.0"

// ==================== 组件生命周期 API ====================

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 创建组件实例
 * @param componentId 组件ID（如 "picmatch"）
 * @param basePath 组件基础路径
 * @return 组件句柄，失败返回 NULL
 */
ComponentHandle Component_Create(const char* componentId, const char* basePath);

/**
 * @brief 销毁组件实例
 * @param handle 组件句柄
 */
void Component_Destroy(ComponentHandle handle);

/**
 * @brief 初始化组件
 * @param handle 组件句柄
 * @param qmlEngine QML引擎实例指针
 * @return 是否成功（1=成功，0=失败）
 */
int Component_Initialize(ComponentHandle handle, void* qmlEngine);

/**
 * @brief 关闭组件
 * @param handle 组件句柄
 */
void Component_Shutdown(ComponentHandle handle);

// ==================== 组件信息获取 API ====================

/**
 * @brief 获取组件完整信息结构体
 * @param handle 组件句柄
 * @param manifest 输出参数，用于接收组件信息
 * @return 是否成功（1=成功，0=失败）
 *
 * 一次性获取组件的所有元数据信息，避免多次 API 调用。
 */
int Component_GetManifest(ComponentHandle handle, ComponentManifest* manifest);

/**
 * @brief 获取组件当前状态
 * @param handle 组件句柄
 * @return 组件状态枚举值
 */
ComponentState Component_GetState(ComponentHandle handle);

// ==================== 组件配置 API ====================

/**
 * @brief 获取组件配置
 * @param handle 组件句柄
 * @param config 输出参数，用于接收配置数据
 * @return 是否成功（1=成功，0=失败）
 */
int Component_GetConfig(ComponentHandle handle, QVariantMap* config);

/**
 * @brief 设置组件配置
 * @param handle 组件句柄
 * @param config 配置数据
 * @return 是否成功（1=成功，0=失败）
 */
int Component_SetConfig(ComponentHandle handle, const QVariantMap* config);

// ==================== QML 集成 API ====================

/**
 * @brief 注册组件的 QML 类型
 * @param handle 组件句柄
 * @param qmlEngine QML引擎实例指针
 * @return 是否成功（1=成功，0=失败）
 */
int Component_RegisterQmlTypes(ComponentHandle handle, void* qmlEngine);

/**
 * @brief 获取组件的 QML 导入路径
 * @param handle 组件句柄
 * @param paths 输出参数，用于接收 QML 导入路径列表
 * @return 是否成功（1=成功，0=失败）
 */
int Component_GetQmlImportPaths(ComponentHandle handle, QStringList* paths);

// ==================== 组件间通信 API ====================

/**
 * @brief 获取组件暴露的接口对象
 * @param handle 组件句柄
 * @param interfaceName 接口名称
 * @return 接口对象指针，失败返回 NULL
 */
void* Component_GetInterface(ComponentHandle handle, const char* interfaceName);

// ==================== 管理器 API ====================

/**
 * @brief 创建组件管理器
 * @return 管理器句柄，失败返回 NULL
 */
ComponentManagerHandle ComponentManager_Create();

/**
 * @brief 销毁组件管理器
 * @param handle 管理器句柄
 */
void ComponentManager_Destroy(ComponentManagerHandle handle);

/**
 * @brief 加载组件
 * @param managerHandle 管理器句柄
 * @param componentId 组件ID
 * @param basePath 组件基础路径
 * @return 组件句柄，失败返回 NULL
 */
ComponentHandle ComponentManager_LoadComponent(ComponentManagerHandle managerHandle,
                                               const char* componentId,
                                               const char* basePath);

/**
 * @brief 卸载组件
 * @param managerHandle 管理器句柄
 * @param componentId 组件ID
 * @return 是否成功（1=成功，0=失败）
 */
int ComponentManager_UnloadComponent(ComponentManagerHandle managerHandle, const char* componentId);

/**
 * @brief 获取组件句柄
 * @param managerHandle 管理器句柄
 * @param componentId 组件ID
 * @return 组件句柄，失败返回 NULL
 */
ComponentHandle ComponentManager_GetComponent(ComponentManagerHandle managerHandle, const char* componentId);

#ifdef __cplusplus
}
#endif

#endif // ICOMPONENT_API_H
