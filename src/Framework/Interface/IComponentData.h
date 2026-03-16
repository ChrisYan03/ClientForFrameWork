/**
 * @file IComponentData.h
 * @brief Framework 组件数据结构定义
 *
 * 此文件定义了 Framework 组件相关的所有数据结构。
 * 纯数据定义，不包含任何实现逻辑。
 */
#ifndef ICOMPONENT_DATA_H
#define ICOMPONENT_DATA_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QStringList>
#include <QVector>
#include <string>
#include <vector>

// ==================== 组件类型枚举 ====================

/**
 * @brief 组件类型枚举
 */
enum ComponentType {
    ComponentType_NativeDll = 0,     //!< 原生DLL/DYLIB组件
    ComponentType_StandaloneExe = 1, //!< 独立可执行程序
    ComponentType_WebUrl = 2,        //!< 网页组件
    ComponentType_EmbeddedExe = 3    //!< 嵌入式组件
};

/**
 * @brief 组件状态枚举
 */
enum ComponentState {
    ComponentState_Unloaded = 0,     //!< 未加载
    ComponentState_Loading = 1,      //!< 加载中
    ComponentState_Loaded = 2,       //!< 已加载
    ComponentState_Initializing = 3, //!< 初始化中
    ComponentState_Running = 4,      //!< 运行中
    ComponentState_Error = 5,        //!< 错误状态
    ComponentState_Shutdown = 6      //!< 已关闭
};

// ==================== 句柄类型定义 ====================

/**
 * @brief 组件句柄类型
 *
 * 用于在 API 中传递组件实例，避免暴露具体类定义。
 */
typedef void* ComponentHandle;

// ==================== 组件元数据结构 ====================

/**
 * @brief 组件元数据结构
 *
 * 用于存储从 manifest.json 解析出的组件配置信息。
 * 此结构体不包含任何 xpack 相关代码，避免与 Qt MOC 冲突。
 */
struct ComponentManifest {
    std::string id;                          //!< 组件唯一标识
    std::string name;                        //!< 组件显示名称
    std::string version;                     //!< 组件版本号
    std::string description;                 //!< 组件描述
    std::string author;                      //!< 组件作者
    std::string type;                        //!< 组件类型（字符串形式）
    std::string icon;                        //!< 图标文件名
    std::string qmlPage;                     //!< 主QML页面路径
    std::string dataPath;                    //!< 数据目录路径
    std::vector<std::string> dependencies;   //!< 依赖组件列表
};

// ==================== 辅助函数声明 ====================

/**
 * @brief 从 manifest.json 文件加载组件元数据
 * @param manifestPath manifest.json 文件路径
 * @param manifest 输出参数，用于存储解析后的元数据
 * @return 是否成功加载和解析
 *
 * 实现在 Framework/src 中，使用 xpack 进行 JSON 解析。
 * 外部组件只需调用此函数，无需关心具体解析实现。
 */
bool loadComponentManifest(const QString &manifestPath, ComponentManifest &manifest);

/**
 * @brief 将组件元数据转换为 QVariantMap（用于 QML 交互）
 * @param manifest 组件元数据
 * @return QVariantMap 表示的元数据
 */
QVariantMap manifestToVariantMap(const ComponentManifest &manifest);

// ==================== 管理器类型定义 ====================

/**
 * @brief 组件管理器句柄
 *
 * 用于在内部管理组件实例，对外部不暴露具体类型。
 */
typedef void* ComponentManagerHandle;

#endif // ICOMPONENT_DATA_H
