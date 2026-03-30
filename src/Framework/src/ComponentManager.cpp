/**
 * @file ComponentManager.cpp
 * @brief 增强版组件管理器实现
 *
 * 支持4种组件类型的统一管理
 */
#include "ComponentManager.h"
#include "../Interface/IComponent.h"
#include "ComponentLoaderFactory.h"
#include <QFile>
#include <QDir>
#include "LogUtil.h"

// ==================== ComponentInstanceV2 实现 ====================

ComponentInstanceV2::ComponentInstanceV2(const QString &componentId, QObject *parent)
    : QObject(parent)
{
    manifest.id = componentId.toStdString();
}

ComponentInstanceV2::~ComponentInstanceV2()
{
    // 清理接口对象
    qDeleteAll(interfaces);
    interfaces.clear();

    // 清理组件对象
    if (componentObject) {
        // 如果是IComponent接口，调用shutdown
        IComponent* component = dynamic_cast<IComponent*>(componentObject);
        if (component) {
            component->shutdown();
        }
        delete componentObject;
        componentObject = nullptr;
    }

    // 清理动态库（仅对NativeDll组件）
    if (loader && componentType == ComponentType_NativeDll) {
        loader->unload(componentObject);
    }
}

// ==================== ComponentManagerV2 实现 ====================

ComponentManagerV2::ComponentManagerV2(QObject *parent)
    : QObject(parent)
{
    // 创建加载器工厂
    m_loaderFactory = new ComponentLoaderFactory(this);
}

ComponentManagerV2::~ComponentManagerV2()
{
    // 卸载所有组件
    for (auto &component : m_components) {
        if (component) {
            component->state = ComponentInstanceV2::Shutdown;
        }
    }
    m_components.clear();
}

ComponentType ComponentManagerV2::parseComponentType(const ComponentManifest& manifest) const
{
    return stringToComponentType(manifest.type);
}

ComponentType ComponentManagerV2::stringToComponentType(const std::string& typeStr) const
{
    if (typeStr == "native" || typeStr == "dll") {
        return ComponentType_NativeDll;
    } else if (typeStr == "exe" || typeStr == "executable") {
        return ComponentType_StandaloneExe;
    } else if (typeStr == "web" || typeStr == "url") {
        return ComponentType_WebUrl;
    } else if (typeStr == "embedded") {
        return ComponentType_EmbeddedExe;
    } else {
        // 默认为原生DLL
        return ComponentType_NativeDll;
    }
}

ComponentInstanceV2* ComponentManagerV2::loadComponent(const QString &componentId, const QString& basePath)
{
    // 检查是否已存在
    if (m_components.contains(componentId)) {
        return m_components[componentId].get();
    }

    // 创建新组件实例
    auto component = QSharedPointer<ComponentInstanceV2>::create(componentId, this);
    component->state = ComponentInstanceV2::Loading;
    component->basePath = basePath;

    // 解析 manifest.json
    QString manifestPath = basePath + QStringLiteral("/meta_info/manifest.json");
    if (!loadComponentManifest(manifestPath, component->manifest)) {
        // 解析失败，使用默认值
        component->manifest.id = componentId.toStdString();
        component->manifest.name = componentId.toStdString();
        component->manifest.version = "1.0.0";
        component->manifest.type = "native";
    }

    // 解析组件类型
    component->componentType = parseComponentType(component->manifest);

    // 检查组件类型是否支持
    if (!isComponentTypeSupported(component->componentType)) {
        LOG_WARN("Unsupported component type: {} for component: {}",
                static_cast<int>(component->componentType), componentId.toStdString());
        component->state = ComponentInstanceV2::Error;
        return nullptr;
    }

    // 获取对应的加载器
    IComponentLoader* loader = m_loaderFactory->getLoader(component->componentType);
    if (!loader) {
        LOG_WARN("Failed to get loader for component type: {}",
                static_cast<int>(component->componentType));
        component->state = ComponentInstanceV2::Error;
        return nullptr;
    }

    // 检查组件是否可以加载
    LOG_INFO("ComponentManager: Checking if component can be loaded: {}", componentId.toStdString());
    if (!loader->canLoad(component->manifest, basePath)) {
        LOG_WARN("Component cannot be loaded: {}", componentId.toStdString());
        component->state = ComponentInstanceV2::Error;
        emit componentLoadFailed(componentId, "Component cannot be loaded");
        return nullptr;
    }
    LOG_INFO("ComponentManager: Component can be loaded: {}", componentId.toStdString());

    // 加载组件
    QObject* componentObject = loader->load(component->manifest, basePath);
    if (!componentObject) {
        LOG_WARN("Failed to load component: {}", componentId.toStdString());
        component->state = ComponentInstanceV2::Error;
        emit componentLoadFailed(componentId, "Failed to load component");
        return nullptr;
    }

    // 保存组件对象和加载器
    component->componentObject = componentObject;
    component->loader = loader;

    // 对于NativeDll组件，尝试转换为IComponent接口
    if (component->componentType == ComponentType_NativeDll) {
        component->iComponent = dynamic_cast<IComponent*>(componentObject);

        // 如果组件实现了 IComponent 接口，可以通过 getInterface 获取更多信息
        // setManifest 是可选的，组件可以通过 getInterface("manifest") 获取 manifest
    }

    component->state = ComponentInstanceV2::Loaded;
    m_components[componentId] = component;

    emit componentLoaded(componentId);

    return component.get();
}

bool ComponentManagerV2::unloadComponent(const QString &componentId)
{
    if (!m_components.contains(componentId)) {
        return false;
    }

    auto &component = m_components[componentId];
    // 使用加载器卸载组件
    if (component->loader) {
        component->loader->unload(component->componentObject);
        component->componentObject = nullptr;
    } else {
        // 兼容旧方式：直接删除组件对象
        if (component->componentObject) {
            IComponent* iComponent = dynamic_cast<IComponent*>(component->componentObject);
            if (iComponent) {
                iComponent->shutdown();
            }
            delete component->componentObject;
            component->componentObject = nullptr;
        }
    }

    component->state = ComponentInstanceV2::Shutdown;
    m_components.remove(componentId);

    emit componentUnloaded(componentId);

    return true;
}

ComponentInstanceV2* ComponentManagerV2::getComponent(const QString &componentId) const
{
    return m_components.value(componentId, nullptr).get();
}

QStringList ComponentManagerV2::getLoadedComponentIds() const
{
    return m_components.keys();
}

QStringList ComponentManagerV2::getComponentsByType(ComponentType type) const
{
    QStringList result;
    for (auto it = m_components.begin(); it != m_components.end(); ++it) {
        if (it.value()->componentType == type) {
            result.append(it.key());
        }
    }
    return result;
}

bool ComponentManagerV2::isComponentTypeSupported(ComponentType type) const
{
    return m_loaderFactory->isTypeSupported(type);
}
