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

enum ComponentType {
    ComponentType_Invalid = -1,
    ComponentType_NativeDll = 0,
    ComponentType_StandaloneExe = 1,
    ComponentType_WebUrl = 2,
    ComponentType_EmbeddedExe = 3
};

enum ComponentState {
    ComponentState_Unloaded = 0,
    ComponentState_Loading = 1,
    ComponentState_Loaded = 2,
    ComponentState_Initializing = 3,
    ComponentState_Running = 4,
    ComponentState_Error = 5,
    ComponentState_Shutdown = 6
};

struct ComponentManifest {
    std::string id;
    std::string name;
    std::string version;
    std::string description;
    std::string author;
    std::string type;
    std::string icon;
    std::string dataPath;
    std::vector<std::string> dependencies;

    std::string module;
    std::string qmlPage;

    std::string executable;
    std::string workingDir;
    std::string arguments;
    bool autoStart = true;

    std::string url;
    bool enableDevTools = false;

    std::string embedMode;
};

bool loadComponentManifest(const QString& manifestPath, ComponentManifest& manifest);

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
