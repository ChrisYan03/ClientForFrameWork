/**
 * @file ComponentTypes.h
 * @brief 组件类型定义
 *
 * 定义了组件系统使用的所有类型和枚举
 * 这是主框架和组件都需要包含的基础头文件
 */
#ifndef FRAMEWORK_COMPONENT_TYPES_H
#define FRAMEWORK_COMPONENT_TYPES_H

#include <QString>
#include <QVariantMap>
#include <QVector>
#include <string>
#include <vector>

// ==================== 组件类型枚举 ====================

/**
 * @brief 组件类型枚举
 */
enum ComponentType {
    ComponentType_Invalid = -1,        //!< 无效类型
    ComponentType_NativeDll = 0,       //!< 原生DLL/DYLIB组件
    ComponentType_StandaloneExe = 1,   //!< 独立可执行程序
    ComponentType_WebUrl = 2,          //!< 网页组件
    ComponentType_EmbeddedExe = 3      //!< 嵌入式组件
};

/**
 * @brief 组件状态枚举
 */
enum ComponentState {
    ComponentState_Unloaded = 0,       //!< 未加载
    ComponentState_Loading = 1,        //!< 加载中
    ComponentState_Loaded = 2,         //!< 已加载
    ComponentState_Initializing = 3,   //!< 初始化中
    ComponentState_Running = 4,        //!< 运行中
    ComponentState_Error = 5,          //!< 错误状态
    ComponentState_Shutdown = 6        //!< 已关闭
};

// ==================== 组件元数据结构 ====================

/**
 * @brief 组件元数据结构
 *
 * 用于存储从 manifest.json 解析出的组件配置信息。
 * 主框架和组件都可以使用此结构访问组件信息。
 */
struct ComponentManifest {
    // 基础信息 (所有组件类型通用)
    std::string id;                          //!< 组件唯一标识
    std::string name;                        //!< 组件显示名称
    std::string version;                     //!< 组件版本号
    std::string description;                 //!< 组件描述
    std::string author;                      //!< 组件作者
    std::string type;                        //!< 组件类型字符串
    std::string icon;                        //!< 图标文件名
    std::string dataPath;                    //!< 数据目录路径
    std::vector<std::string> dependencies;   //!< 依赖组件列表

    // NativeDll特有字段
    std::string module;                      //!< 动态库文件名
    std::string qmlPage;                     //!< 主QML页面路径

    // StandaloneExe特有字段
    std::string executable;                  //!< 可执行文件名
    std::string workingDir;                  //!< 工作目录
    std::string arguments;                   //!< 命令行参数
    bool autoStart = true;                   //!< 是否自动启动

    // WebUrl特有字段
    std::string url;                         //!< Web页面URL
    bool enableDevTools = false;             //!< 是否启用开发者工具

    // EmbeddedExe特有字段
    std::string embedMode;                   //!< 嵌入模式
};

// ==================== 辅助函数声明 ====================

/**
 * @brief 从 manifest.json 文件加载组件元数据
 * @param manifestPath manifest.json 文件路径
 * @param manifest 输出参数，用于存储解析后的元数据
 * @return 是否成功加载和解析
 */
bool loadComponentManifest(const QString& manifestPath, ComponentManifest& manifest);

// ==================== 辅助函数 ====================

/**
 * @brief 字符串转换为组件类型
 * @param typeStr 类型字符串
 * @return 组件类型枚举
 */
inline ComponentType stringToComponentType(const std::string& typeStr) {
    if (typeStr == "native" || typeStr == "dll") {
        return ComponentType_NativeDll;
    } else if (typeStr == "exe" || typeStr == "executable") {
        return ComponentType_StandaloneExe;
    } else if (typeStr == "web" || typeStr == "url") {
        return ComponentType_WebUrl;
    } else if (typeStr == "embedded") {
        return ComponentType_EmbeddedExe;
    }
    return ComponentType_Invalid;
}

/**
 * @brief 组件类型转换为字符串
 * @param type 组件类型枚举
 * @return 类型字符串
 */
inline std::string componentTypeToString(ComponentType type) {
    switch (type) {
        case ComponentType_NativeDll: return "native";
        case ComponentType_StandaloneExe: return "exe";
        case ComponentType_WebUrl: return "web";
        case ComponentType_EmbeddedExe: return "embedded";
        default: return "unknown";
    }
}

#endif // FRAMEWORK_COMPONENT_TYPES_H