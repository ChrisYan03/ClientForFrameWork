/**
 * @file ComponentApiImpl.cpp
 * @brief C 风格 API 接口实现
 *
 * 实现 ComponentApi.h 中声明的所有 C 风格函数。
 */
#include "ComponentManager.h"
#include "ComponentApi.h"
#include "IComponentData.h"
#include <QMetaObject>
#include <QQmlEngine>
#include <cstring>

// ==================== 组件生命周期 API 实现 ====================

extern "C" {

ComponentHandle Component_Create(const char* componentId, const char* basePath)
{
    if (!componentId) return nullptr;

    // 创建临时管理器
    ComponentManager manager;
    QString id = QString::fromUtf8(componentId);
    QString path = basePath ? QString::fromUtf8(basePath) : QString();

    auto instance = manager.loadComponent(id, path);
    return static_cast<ComponentHandle>(instance);
}

void Component_Destroy(ComponentHandle handle)
{
    // 组件由管理器统一管理，这里不做实际销毁
}

int Component_Initialize(ComponentHandle handle, void* qmlEngine)
{
    auto instance = static_cast<ComponentInstance*>(handle);
    if (!instance) return 0;

    instance->qmlEngine = static_cast<QQmlEngine*>(qmlEngine);
    instance->state = ComponentInstance::Initializing;

    // 如果组件实现了 IComponent 接口，调用其 initialize 方法
    if (instance->iComponent) {
        QString basePath = instance->basePath;
        bool success = instance->iComponent->initialize(instance->qmlEngine, basePath);
        if (!success) {
            instance->state = ComponentInstance::Error;
            return 0;
        }
    }

    instance->state = ComponentInstance::Running;
    return 1;
}

void Component_Shutdown(ComponentHandle handle)
{
    auto instance = static_cast<ComponentInstance*>(handle);
    if (!instance) return;

    // 如果组件实现了 IComponent 接口，调用其 shutdown 方法
    if (instance->iComponent) {
        instance->iComponent->shutdown();
    }

    instance->state = ComponentInstance::Shutdown;
}

// ==================== 组件信息获取 API 实现 ====================

int Component_GetManifest(ComponentHandle handle, ComponentManifest* manifest)
{
    auto instance = static_cast<ComponentInstance*>(handle);
    if (!instance || !manifest) return 0;

    // 从 ComponentInstance 的 manifest 成员获取信息
    // 组件通过 m_manifest 成员直接访问
    *manifest = instance->manifest;
    return 1;
}

ComponentState Component_GetState(ComponentHandle handle)
{
    auto instance = static_cast<ComponentInstance*>(handle);
    if (!instance) return ComponentState_Unloaded;

    switch (instance->state) {
        case ComponentInstance::Unloaded: return ComponentState_Unloaded;
        case ComponentInstance::Loading: return ComponentState_Loading;
        case ComponentInstance::Loaded: return ComponentState_Loaded;
        case ComponentInstance::Initializing: return ComponentState_Initializing;
        case ComponentInstance::Running: return ComponentState_Running;
        case ComponentInstance::Error: return ComponentState_Error;
        case ComponentInstance::Shutdown: return ComponentState_Shutdown;
        default: return ComponentState_Unloaded;
    }
}

// ==================== 组件配置 API 实现 ====================

int Component_GetConfig(ComponentHandle handle, QVariantMap* config)
{
    auto instance = static_cast<ComponentInstance*>(handle);
    if (!instance || !config) return 0;

    *config = instance->config;
    return 1;
}

int Component_SetConfig(ComponentHandle handle, const QVariantMap* config)
{
    auto instance = static_cast<ComponentInstance*>(handle);
    if (!instance || !config) return 0;

    instance->config = *config;
    return 1;
}

// ==================== QML 集成 API 实现 ====================

int Component_RegisterQmlTypes(ComponentHandle handle, void* qmlEngine)
{
    auto instance = static_cast<ComponentInstance*>(handle);
    if (!instance || !qmlEngine) return 0;

    QQmlEngine* engine = static_cast<QQmlEngine*>(qmlEngine);

    // 优先使用 IComponent 接口
    if (instance->iComponent) {
        instance->iComponent->registerQmlTypes(engine);
        return 1;
    }

    // 兼容旧方式：通过 QMetaObject 调用
    if (instance->componentObject) {
        bool ok = QMetaObject::invokeMethod(instance->componentObject, "registerQmlTypes",
                                            Q_ARG(QQmlEngine*, engine));
        return ok ? 1 : 0;
    }

    return 0;
}

int Component_GetQmlImportPaths(ComponentHandle handle, QStringList* paths)
{
    auto instance = static_cast<ComponentInstance*>(handle);
    if (!instance || !paths) return 0;

    *paths = QStringList();
    return 1;
}

// ==================== 组件间通信 API 实现 ====================

void* Component_GetInterface(ComponentHandle handle, const char* interfaceName)
{
    auto instance = static_cast<ComponentInstance*>(handle);
    if (!instance || !interfaceName) return nullptr;

    QString name = QString::fromUtf8(interfaceName);

    // 优先使用 IComponent 接口
    if (instance->iComponent) {
        return instance->iComponent->getInterface(name);
    }

    // 兼容旧方式：从 interfaces map 中获取
    return instance->interfaces.value(name, nullptr);
}

// ==================== 管理器 API 实现 ====================

ComponentManagerHandle ComponentManager_Create()
{
    return new ComponentManager();
}

void ComponentManager_Destroy(ComponentManagerHandle handle)
{
    auto manager = static_cast<ComponentManager*>(handle);
    delete manager;
}

ComponentHandle ComponentManager_LoadComponent(ComponentManagerHandle managerHandle,
                                               const char* componentId,
                                               const char* basePath)
{
    auto manager = static_cast<ComponentManager*>(managerHandle);
    if (!manager || !componentId) return nullptr;

    QString id = QString::fromUtf8(componentId);
    QString path = basePath ? QString::fromUtf8(basePath) : QString();

    auto instance = manager->loadComponent(id, path);
    return static_cast<ComponentHandle>(instance);
}

int ComponentManager_UnloadComponent(ComponentManagerHandle managerHandle, const char* componentId)
{
    auto manager = static_cast<ComponentManager*>(managerHandle);
    if (!manager || !componentId) return 0;

    QString id = QString::fromUtf8(componentId);
    return manager->unloadComponent(id) ? 1 : 0;
}

ComponentHandle ComponentManager_GetComponent(ComponentManagerHandle managerHandle, const char* componentId)
{
    auto manager = static_cast<ComponentManager*>(managerHandle);
    if (!manager || !componentId) return nullptr;

    QString id = QString::fromUtf8(componentId);
    auto instance = manager->getComponent(id);
    return static_cast<ComponentHandle>(instance);
}

} // extern "C"
