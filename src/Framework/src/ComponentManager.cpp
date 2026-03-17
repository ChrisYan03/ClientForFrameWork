/**
 * @file ComponentManager.cpp
 * @brief 组件管理器实现
 */
#include "ComponentManager.h"
#include "../Interface/IComponentData.h"
#include "../Interface/IComponent.h"
#include <QFile>
#include <QDir>
#include <QLibrary>

// ==================== ComponentInstance 实现 ====================

ComponentInstance::ComponentInstance(const QString &componentId, QObject *parent)
    : QObject(parent)
{
    manifest.id = componentId.toStdString();
}

ComponentInstance::~ComponentInstance()
{
    // 清理接口对象
    qDeleteAll(interfaces);
    interfaces.clear();

    // 卸载动态库
    if (library) {
        library->unload();
        delete library;
        library = nullptr;
    }
}

// ==================== ComponentManager 实现 ====================

ComponentManager::ComponentManager(QObject *parent)
    : QObject(parent)
{
}

ComponentManager::~ComponentManager()
{
    // 卸载所有组件
    for (auto &component : m_components) {
        if (component) {
            component->state = ComponentInstance::Shutdown;
        }
    }
    m_components.clear();
}

ComponentInstance* ComponentManager::loadComponent(const QString &componentId, const QString &basePath)
{
    // 检���是否已存在
    if (m_components.contains(componentId)) {
        return m_components[componentId].get();
    }

    // 创建新组件实例
    auto component = QSharedPointer<ComponentInstance>::create(componentId, this);
    component->state = ComponentInstance::Loading;
    component->basePath = basePath;

    // 解析 manifest.json
    QString manifestPath = basePath + QStringLiteral("/meta_info/manifest.json");
    if (!loadComponentManifest(manifestPath, component->manifest)) {
        // 解析失败，使用默认值
        component->manifest.id = componentId.toStdString();
        component->manifest.name = componentId.toStdString();
        component->manifest.version = "1.0.0";
        component->manifest.type = "native";
        component->state = ComponentInstance::Error;
        return nullptr;
    }

    // 构建动态库路径
    QString libPath = basePath + QStringLiteral("/bin/");
    QString module = QString::fromStdString(component->manifest.type == "native" ? "component" :
                                                     component->manifest.type.c_str());

#if defined(Q_OS_WIN)
    libPath += module + QStringLiteral(".dll");
#elif defined(Q_OS_MAC)
    libPath += QStringLiteral("lib") + module + QStringLiteral(".dylib");
#else
    libPath += QStringLiteral("lib") + module + QStringLiteral(".so");
#endif

    // 加载动态库
    component->library = new QLibrary(libPath);
    if (!component->library->load()) {
        // 尝试其他可能的模块名
        QString altModule = QString::fromStdString(component->manifest.id) + QStringLiteral("Component");
#if defined(Q_OS_WIN)
        libPath = basePath + QStringLiteral("/bin/") + altModule + QStringLiteral(".dll");
#elif defined(Q_OS_MAC)
        libPath = basePath + QStringLiteral("/bin/lib") + altModule + QStringLiteral(".dylib");
#else
        libPath = basePath + QStringLiteral("/bin/lib") + altModule + QStringLiteral(".so");
#endif

        delete component->library;
        component->library = new QLibrary(libPath);
        if (!component->library->load()) {
            component->state = ComponentInstance::Error;
            return nullptr;
        }
    }

    // 解析 createComponent 导出函数
    typedef void* (*CreateComponentFunc)();
    auto createFn = reinterpret_cast<CreateComponentFunc>(
        component->library->resolve("createComponent")
    );

    if (!createFn) {
        component->state = ComponentInstance::Error;
        return nullptr;
    }

    // 调用工厂函数创建组件实例
    void* componentVoid = createFn();
    if (!componentVoid) {
        component->state = ComponentInstance::Error;
        return nullptr;
    }

    // 存储组件实例指针（作为 QObject*）
    component->componentObject = static_cast<QObject*>(componentVoid);

    // 尝试转换为 IComponent 接口（运行时多态转换，无需 IComponent 继承 QObject）
    component->iComponent = dynamic_cast<IComponent*>(component->componentObject);

    // 如果组件实现了 IComponent，将 manifest 信息传递给组件
    if (component->iComponent && component->componentObject) {
        // 通过 QMetaObject 调用 setManifest 方法传递 manifest
        QVariant arg = QVariant::fromValue(component->manifest);
        QMetaObject::invokeMethod(component->componentObject, "setManifest",
                                 Qt::DirectConnection,
                                 Q_ARG(ComponentManifest, component->manifest));
    }

    component->state = ComponentInstance::Loaded;
    m_components[componentId] = component;

    return component.get();
}

bool ComponentManager::unloadComponent(const QString &componentId)
{
    if (!m_components.contains(componentId)) {
        return false;
    }

    auto &component = m_components[componentId];

    // 关闭组件
    if (component->componentObject) {
        // 这里可以调用组件的 shutdown 方法
        component->componentObject = nullptr;
    }

    component->state = ComponentInstance::Shutdown;
    m_components.remove(componentId);

    return true;
}

ComponentInstance* ComponentManager::getComponent(const QString &componentId) const
{
    return m_components.value(componentId, nullptr).get();
}

QStringList ComponentManager::getLoadedComponentIds() const
{
    return m_components.keys();
}
