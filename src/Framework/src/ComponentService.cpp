/**
 * @file ComponentService.cpp
 * @brief Framework 组件服务实现
 */
#include "ComponentService.h"
#include "ComponentApi.h"
#include "IComponentData.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QQmlContext>
#include <QUrl>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(ComponentServiceLog, "ComponentService")

// 尝试多个可能的配置路径
static QStringList getConfigPaths(const QString &baseDir)
{
    QStringList paths;
    QDir dir(baseDir);

    // 尝试不同的配置路径
    paths << dir.absoluteFilePath("config/components.json");
    paths << dir.absoluteFilePath("config/component.json");

#if defined(Q_OS_MAC)
    paths << dir.absoluteFilePath("Release/config/components.json");
    paths << dir.absoluteFilePath("Release/config/component.json");
#endif

    return paths;
}

// 读取配置文件内容，返回找到的路径
static QByteArray tryReadConfig(const QStringList &paths, QString &foundPath)
{
    for (const QString &path : paths) {
        QFile f(path);
        if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            foundPath = path;
            return f.readAll();
        }
    }
    return QByteArray();
}

ComponentService::ComponentService(QObject *parent)
    : QObject(parent)
{
}

ComponentService::~ComponentService()
{
    shutdown();
}

bool ComponentService::initialize(QQmlEngine *engine, QObject *appController)
{
    if (m_initialized) {
        qWarning() << "ComponentService already initialized";
        return true;
    }

    if (!engine || !appController) {
        qWarning() << "Invalid parameters for ComponentService::initialize";
        return false;
    }

    m_engine = engine;
    m_appController = appController;

    // 创建组件管理器
    m_manager = ComponentManager_Create();
    if (!m_manager) {
        qWarning() << "Failed to create ComponentManager";
        return false;
    }

    // 获取启用的组件列表
    QStringList enabledComponents = loadEnabledComponents();
    qInfo() << "Loading" << enabledComponents.size() << "components";

    // 加载每个组件
    bool allSuccess = true;
    for (const QString &componentId : enabledComponents) {
        if (!loadComponent(componentId, m_basePath + "/Component/" + componentId)) {
            allSuccess = false;
            emit componentLoadFailed(componentId, "Failed to load component");
        } else {
            // 获取组件信息并注册到 QML
            ComponentManifest manifest;
            if (getComponentManifest(componentId, manifest)) {
                registerComponentToQml(componentId, manifest, engine, appController);
            }
            emit componentLoaded(componentId);
        }
    }

    m_initialized = true;
    emit allComponentsLoaded(allSuccess);

    return true;
}

void ComponentService::shutdown()
{
    if (!m_initialized) {
        return;
    }

    qInfo() << "Shutting down ComponentService";

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

    // 销毁组件管理器
    if (m_manager) {
        ComponentManager_Destroy(m_manager);
        m_manager = nullptr;
    }

    m_initialized = false;
}

// 查找组件目录（处理大小写问题）
// basePath 已经是完整路径，但可能大小写不匹配
static QString findComponentDir(const QString &basePath)
{
    // 首先检查路径是否直接存在
    if (QDir(basePath).exists()) {
        return basePath;
    }

    // 路径不存在，尝试在父目录中查找大小写不同的同名目录
    QDir parentDir = QFileInfo(basePath).dir();
    QString dirName = QFileInfo(basePath).fileName();

    if (!parentDir.exists()) {
        return basePath;
    }

    QStringList entries = parentDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &entry : entries) {
        if (entry.compare(dirName, Qt::CaseInsensitive) == 0) {
            return parentDir.absoluteFilePath(entry);
        }
    }

    return basePath;  // 返回原始路径，让后续代码报错
}

bool ComponentService::loadComponent(const QString &componentId, const QString &basePath)
{
    if (!m_manager) {
        qWarning() << "ComponentManager is null";
        return false;
    }

    // 查找实际的组件目录（处理大小写问题）
    QString actualComponentPath = findComponentDir(basePath);
    qInfo() << "Loading component:" << componentId << "from" << actualComponentPath;

    // 使用 Framework API 加载组件
    ComponentHandle component = ComponentManager_LoadComponent(
        m_manager,
        componentId.toUtf8().constData(),
        actualComponentPath.toUtf8().constData()
    );

    if (!component) {
        qWarning() << "Failed to load component:" << componentId;
        return false;
    }

    // 初始化组件
    if (!Component_Initialize(component, m_engine)) {
        qWarning() << "Failed to initialize component:" << componentId;
        ComponentManager_UnloadComponent(m_manager, componentId.toUtf8().constData());
        return false;
    }

    // 注册 QML 类型
    Component_RegisterQmlTypes(component, m_engine);

    // 存储组件句柄
    m_components[componentId] = component;

    // 获取组件信息用于日志
    ComponentManifest manifest;
    if (Component_GetManifest(component, &manifest)) {
        qInfo() << "Component loaded successfully:" << QString::fromStdString(manifest.name)
                << "v" << QString::fromStdString(manifest.version);
    }

    return true;
}

bool ComponentService::unloadComponent(const QString &componentId)
{
    if (!m_components.contains(componentId)) {
        return false;
    }

    ComponentHandle component = m_components[componentId];

    // 关闭组件
    Component_Shutdown(component);

    // 卸载组件
    bool success = ComponentManager_UnloadComponent(m_manager, componentId.toUtf8().constData()) != 0;

    if (success) {
        m_components.remove(componentId);
        emit componentUnloaded(componentId);
    }

    return success;
}

ComponentHandle ComponentService::getComponent(const QString &componentId) const
{
    return m_components.value(componentId, nullptr);
}

bool ComponentService::getComponentManifest(const QString &componentId, ComponentManifest &manifest) const
{
    ComponentHandle component = getComponent(componentId);
    if (!component) {
        return false;
    }

    return Component_GetManifest(component, &manifest) != 0;
}

QStringList ComponentService::getLoadedComponentIds() const
{
    return m_components.keys();
}

bool ComponentService::isComponentLoaded(const QString &componentId) const
{
    return m_components.contains(componentId);
}

void ComponentService::setBasePath(const QString &basePath)
{
    m_basePath = basePath;
}

QString ComponentService::basePath() const
{
    return m_basePath;
}

QStringList ComponentService::loadEnabledComponents()
{
    // 尝试多个可能的配置路径
    QStringList configPaths = getConfigPaths(m_basePath);
    QString foundConfigPath;
    QByteArray jsonData = tryReadConfig(configPaths, foundConfigPath);

    if (jsonData.isEmpty()) {
        qWarning() << "Component config not found (tried:" << configPaths << ")";
        return QStringList();
    }

    // 如果配置在 Release 子目录中，更新 basePath
    if (foundConfigPath.contains("/Release/config/")) {
        QDir dir(m_basePath);
        m_basePath = dir.absoluteFilePath("Release");
        qInfo() << "Config found in Release, updated basePath to:" << m_basePath;
    }

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "Failed to parse component config:" << err.errorString();
        return QStringList();
    }

    QJsonArray componentsArray = doc.object().value("components").toArray();
    QStringList componentList;
    for (const QJsonValue &value : componentsArray) {
        QString componentId = value.toString().trimmed();
        if (!componentId.isEmpty()) {
            componentList.append(componentId);
        }
    }

    qInfo() << "Base dir:" << m_basePath << ", components:" << componentList.size();
    return componentList;
}

bool ComponentService::registerComponentToQml(const QString &componentId,
                                            const ComponentManifest &manifest,
                                            QQmlEngine *engine,
                                            QObject *appController)
{
    // 查找实际的组件目录（处理大小写问题）
    QString componentPath = findComponentDir(m_basePath + "/Component/" + componentId);

    // 注册组件图标
    QString iconPath = componentPath + "/meta_info/" + QString::fromStdString(manifest.icon);

    // 调用 appController 的方法（通过 QMetaObject）
    QMetaObject::invokeMethod(appController, "registerComponentIcon",
                             Q_ARG(QString, QString::fromStdString(manifest.id)),
                             Q_ARG(QString, iconPath));

    // 注册组件页面（如果有）
    if (!manifest.qmlPage.empty()) {
        QString qmlPath = componentPath + "/bin/qml/" + QString::fromStdString(manifest.qmlPage);
        engine->addImportPath(componentPath + "/bin/qml");
        QMetaObject::invokeMethod(appController, "registerComponentPage",
                                 Q_ARG(QString, QString::fromStdString(manifest.id)),
                                 Q_ARG(QUrl, QUrl::fromLocalFile(qmlPath)));
    }

    qInfo() << "Registered component to QML:" << QString::fromStdString(manifest.name);
    return true;
}
