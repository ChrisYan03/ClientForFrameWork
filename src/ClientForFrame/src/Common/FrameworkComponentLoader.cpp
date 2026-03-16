/**
 * @file FrameworkComponentLoader.cpp
 * @brief 基于 Framework 的组件加载器实现
 */
#include "FrameworkComponentLoader.h"
#include "ApplicationPaths.h"
#include "QmlBridge/AppController.h"
#include "LogUtil.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QQmlContext>
#include <QUrl>

FrameworkComponentLoader::FrameworkComponentLoader(QObject *parent)
    : QObject(parent)
{
    // 创建 Framework 组件管理器
    m_manager = ComponentManager_Create();
    if (!m_manager) {
        LOG_ERROR("Failed to create Framework component manager");
    }
}

FrameworkComponentLoader::~FrameworkComponentLoader()
{
    unloadAllComponents();
    if (m_manager) {
        ComponentManager_Destroy(m_manager);
        m_manager = nullptr;
    }
}

bool FrameworkComponentLoader::loadAllComponents(QQmlEngine *engine, QObject *appController)
{
    if (!m_manager || !engine || !appController) {
        LOG_ERROR("Invalid parameters for loadAllComponents");
        return false;
    }

    // 获取启用的组件列表
    QStringList enabledComponents = loadEnabledComponents();
    LOG_INFO("Loading {} components", enabledComponents.size());

    bool allSuccess = true;
    for (const QString &componentId : enabledComponents) {
        // 构建组件路径
        QString componentPath = m_basePath + QStringLiteral("/Component/") + componentId;

        if (!loadComponent(componentId, componentPath, engine)) {
            allSuccess = false;
            emit componentLoadFailed(componentId, "Failed to load component");
        } else {
            // 注册到 QML 系统
            ComponentManifest manifest;
            if (getComponentManifest(componentId, manifest)) {
                registerComponentToQml(componentId, manifest, engine, appController);
            }
            emit componentLoaded(componentId);
        }
    }

    return allSuccess;
}

bool FrameworkComponentLoader::loadComponent(const QString &componentId, const QString &basePath, QQmlEngine *engine)
{
    if (!m_manager) {
        LOG_ERROR("Component manager is null");
        return false;
    }

    LOG_INFO("Loading component: {} from {}", componentId.toStdString(), basePath.toStdString());

    // 使用 Framework API 加载组件
    ComponentHandle component = ComponentManager_LoadComponent(
        m_manager,
        componentId.toUtf8().constData(),
        basePath.toUtf8().constData()
    );

    if (!component) {
        LOG_ERROR("Failed to load component: {}", componentId.toStdString());
        return false;
    }

    // 初始化组件
    if (!Component_Initialize(component, engine)) {
        LOG_ERROR("Failed to initialize component: {}", componentId.toStdString());
        ComponentManager_UnloadComponent(m_manager, componentId.toUtf8().constData());
        return false;
    }

    // 存储组件句柄
    m_components[componentId] = component;

    // 获取组件信息用于日志
    ComponentManifest manifest;
    if (Component_GetManifest(component, &manifest)) {
        LOG_INFO("Component loaded successfully: {} v{}",
                 manifest.name.c_str(), manifest.version.c_str());
    }

    return true;
}

void FrameworkComponentLoader::unloadAllComponents()
{
    if (!m_manager) return;

    LOG_INFO("Unloading all components");

    // 卸载所有组件
    for (const QString &componentId : m_components.keys()) {
        ComponentHandle component = m_components[componentId];

        // 关闭组件
        Component_Shutdown(component);

        // 卸载组件
        ComponentManager_UnloadComponent(m_manager, componentId.toUtf8().constData());

        emit componentUnloaded(componentId);
    }

    m_components.clear();
}

ComponentHandle FrameworkComponentLoader::getComponent(const QString &componentId) const
{
    return m_components.value(componentId, nullptr);
}

bool FrameworkComponentLoader::getComponentManifest(const QString &componentId, ComponentManifest &manifest) const
{
    ComponentHandle component = getComponent(componentId);
    if (!component) {
        return false;
    }

    return Component_GetManifest(component, &manifest) != 0;
}

QStringList FrameworkComponentLoader::getLoadedComponentIds() const
{
    return m_components.keys();
}

QStringList FrameworkComponentLoader::loadEnabledComponents()
{
    ApplicationPaths paths;
    QString baseDir = paths.initialBaseDir();
    auto tryOpen = [](const QString &p) -> QByteArray {
        QFile f(p);
        return f.open(QIODevice::ReadOnly | QIODevice::Text) ? f.readAll() : QByteArray();
    };
    QString path = QDir(baseDir).absoluteFilePath(QStringLiteral("config/components.json"));
    QByteArray jsonData = tryOpen(path);
    if (jsonData.isEmpty()) {
        path = QDir(baseDir).absoluteFilePath(QStringLiteral("config/component.json"));
        jsonData = tryOpen(path);
    }
#if defined(Q_OS_MAC)
    if (jsonData.isEmpty()) {
        path = QDir(baseDir).absoluteFilePath(QStringLiteral("Release/config/components.json"));
        jsonData = tryOpen(path);
        if (jsonData.isEmpty()) {
            path = QDir(baseDir).absoluteFilePath(QStringLiteral("Release/config/component.json"));
            jsonData = tryOpen(path);
        }
        if (!jsonData.isEmpty())
            paths.setResolvedBaseDir(QDir(baseDir).absoluteFilePath(QStringLiteral("Release")));
    }
#endif
    if (jsonData.isEmpty()) {
        LOG_ERROR("config not found (tried config/ and Release/config/), baseDir: {}", baseDir.toStdString());
        return QStringList();
    }
    m_basePath = paths.baseDir();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        LOG_ERROR("Failed to parse config: {}", path.toStdString());
        return QStringList();
    }
    QJsonArray componentsArray = doc.object().value(QStringLiteral("components")).toArray();
    QStringList componentList;
    for (const QJsonValue &value : componentsArray) {
        QString componentId = value.toString().trimmed();
        if (!componentId.isEmpty())
            componentList.append(componentId);
    }
    LOG_INFO("baseDir: {}, components: {} (from {})", m_basePath.toStdString(),
             static_cast<int>(componentList.size()), path.toStdString());
    return componentList;
}

bool FrameworkComponentLoader::registerComponentToQml(const QString &componentId,
                                                      const ComponentManifest &manifest,
                                                      QQmlEngine *engine,
                                                      QObject *appController)
{
    auto *controller = qobject_cast<AppController*>(appController);
    if (!controller) {
        LOG_ERROR("Invalid AppController");
        return false;
    }

    // 注册组件图标
    QString iconPath = m_basePath + QStringLiteral("/Component/") + componentId +
                      QStringLiteral("/meta_info/") + QString::fromStdString(manifest.icon);
    controller->registerComponentIcon(QString::fromStdString(manifest.id), iconPath);

    // 注册组件页面（如果有）
    if (!manifest.qmlPage.empty()) {
        QString qmlPath = m_basePath + QStringLiteral("/Component/") + componentId +
                         QStringLiteral("/bin/qml/") + QString::fromStdString(manifest.qmlPage);
        engine->addImportPath(m_basePath + QStringLiteral("/Component/") + componentId + QStringLiteral("/bin/qml"));
        controller->registerComponentPage(
            QString::fromStdString(manifest.id),
            QUrl::fromLocalFile(qmlPath)
        );
    }

    // 注册 QML 类型
    ComponentHandle component = getComponent(componentId);
    if (component) {
        Component_RegisterQmlTypes(component, engine);
    }

    LOG_INFO("Registered component to QML: {}", manifest.name.c_str());
    return true;
}
