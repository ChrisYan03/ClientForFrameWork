/**
 * @file ComponentService.cpp
 * @brief 组件服务实现
 */
#include "../include/ComponentService.h"
#include "ComponentManager.h"
#include "../Interface/IComponent.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QQmlContext>
#include <QCoreApplication>
#include <QEvent>
#include "LogUtil.h"

// ==================== ComponentService::ComponentManagerPrivate 实现 ====================

class ComponentService::ComponentManagerPrivate : public QObject {
    Q_OBJECT

public:
    explicit ComponentManagerPrivate(QObject* parent = nullptr)
        : QObject(parent) {
        manager = new ComponentManagerV2(this);
    }

    ~ComponentManagerPrivate() {
        delete manager;
    }

    ComponentManagerV2* manager = nullptr;
    QString basePath;
    QQmlEngine* qmlEngine = nullptr;
    QObject* appController = nullptr;
};

// ==================== ComponentService 实现 ====================

ComponentService::ComponentService(QObject* parent)
    : QObject(parent)
{
    d = new ComponentManagerPrivate(this);
}

ComponentService::~ComponentService()
{
    shutdown();
    delete d;
}

void ComponentService::setBasePath(const QString& basePath)
{
    d->basePath = basePath;
}

bool ComponentService::initialize(QQmlEngine* engine, QObject* appController)
{
    if (!engine || !appController) {
        LOG_WARN("Invalid parameters for ComponentService::initialize");
        return false;
    }

    d->qmlEngine = engine;
    d->appController = appController;

    // 读取配置文件
    QString configPath = d->basePath + "/config/components.json";
    QFile configFile(configPath);
    if (!configFile.open(QIODevice::ReadOnly)) {
        LOG_WARN("Failed to open config file: {}", configPath.toStdString());
        return false;
    }

    // 解析配置
    QJsonDocument doc = QJsonDocument::fromJson(configFile.readAll());
    if (doc.isNull() || !doc.isObject()) {
        LOG_WARN("Invalid config file format: {}", configPath.toStdString());
        return false;
    }

    QJsonObject rootObj = doc.object();
    QJsonArray componentsArray = rootObj["components"].toArray();

    // 加载启用的组件
    for (const QJsonValue& componentValue : componentsArray) {
        QString componentId;

        // 支持两种格式：
        // 1. {"components": [{"id": "xxx", "enabled": true}]}  - 对象格式
        // 2. {"components": ["xxx", "yyy"]}  - 简单字符串数组格式
        if (componentValue.isString()) {
            componentId = componentValue.toString();
        } else if (componentValue.isObject()) {
            QJsonObject componentObj = componentValue.toObject();
            if (componentObj["enabled"].toBool(true)) {
                componentId = componentObj["id"].toString();
            }
        }

        if (!componentId.isEmpty()) {
            QString componentPath = d->basePath + "/Component/" + componentId;
            LOG_INFO("ComponentService: Loading component {} from {}",
                    componentId.toStdString(), componentPath.toStdString());
            if (!loadComponent(componentId)) {
                LOG_WARN("Failed to load component: {}", componentId.toStdString());
            }
        }
    }

    return true;
}

void ComponentService::shutdown()
{
    if (d->manager) {
        QStringList components = d->manager->getLoadedComponentIds();
        for (const QString& componentId : components) {
            unloadComponent(componentId);
        }
    }
}

bool ComponentService::loadComponent(const QString& componentId)
{
    QString componentPath = d->basePath + "/Component/" + componentId;

    ComponentInstanceV2* instance = d->manager->loadComponent(componentId, componentPath);
    if (!instance) {
        emit componentLoadFailed(componentId, "Failed to load component");
        return false;
    }

    // 初始化NativeDll组件
    LOG_INFO("ComponentService: Initializing NativeDll component: {}", componentId.toStdString());
    if (instance->componentType == ComponentType_NativeDll) {
        IComponent* component = dynamic_cast<IComponent*>(instance->componentObject);
        if (component) {
            LOG_INFO("ComponentService: Calling initialize on component");
            int result = component->initialize(d->qmlEngine, componentPath.toUtf8().constData());
            LOG_INFO("ComponentService: initialize returned: {}", result);
            if (result) {
                LOG_INFO("ComponentService: Registering QML types");
                component->registerQmlTypes(d->qmlEngine);

                // 注册组件页面到 AppController
                QString qmlPage = QString::fromStdString(instance->manifest.qmlPage);
                if (!qmlPage.isEmpty() && d->appController) {
                    // QML文件在 bin/qml/ 目录下
                    // 由于已经添加了 import path，可以直接用相对路径
                    QUrl pageUrl = QUrl::fromLocalFile(componentPath + "/bin/" + qmlPage);
                    LOG_INFO("ComponentService: Registering component page: {}", pageUrl.toString().toStdString());
                    QMetaObject::invokeMethod(d->appController, "registerComponentPage",
                                           Qt::AutoConnection,
                                           Q_ARG(QString, componentId),
                                           Q_ARG(QUrl, pageUrl));
                }

                // 注册组件图标到 AppController
                QString iconFile = QString::fromStdString(instance->manifest.icon);
                if (!iconFile.isEmpty() && d->appController) {
                    // 图标路径：Component/PicMatch/meta_info/runtime_info/face_recognition.svg
                    QString iconPath = componentPath + "/meta_info/runtime_info/" + iconFile;
                    LOG_INFO("ComponentService: Registering component icon: {}", iconPath.toStdString());
                    QMetaObject::invokeMethod(d->appController, "registerComponentIcon",
                                           Qt::AutoConnection,
                                           Q_ARG(QString, componentId),
                                           Q_ARG(QString, iconPath));
                }

                // 注册组件名称到 AppController
                QString componentName = QString::fromStdString(instance->manifest.name);
                if (!componentName.isEmpty() && d->appController) {
                    LOG_INFO("ComponentService: Registering component name: {} -> {}", componentId.toStdString(), componentName.toStdString());
                    QMetaObject::invokeMethod(d->appController, "registerComponentName",
                                           Qt::AutoConnection,
                                           Q_ARG(QString, componentId),
                                           Q_ARG(QString, componentName));
                }

                LOG_INFO("ComponentService: Emitting componentLoaded signal");
                emit componentLoaded(componentId);
                return true;
            } else {
                LOG_WARN("ComponentService: Component initialize failed");
            }
        } else {
            LOG_WARN("ComponentService: Cannot cast to IComponent");
        }
    } else {
        LOG_INFO("ComponentService: Component type is not NativeDll, emitting componentLoaded directly");
        emit componentLoaded(componentId);
        return true;
    }

    return false;
}

bool ComponentService::unloadComponent(const QString& componentId)
{
    bool result = d->manager->unloadComponent(componentId);
    if (result) {
        emit componentUnloaded(componentId);
    }
    return result;
}

QStringList ComponentService::loadedComponents() const
{
    return d->manager->getLoadedComponentIds();
}

QStringList ComponentService::getComponentsByType(ComponentType type) const
{
    return d->manager->getComponentsByType(type);
}

void ComponentService::notifyThemeChanged(int theme)
{
    QStringList components = d->manager->getLoadedComponentIds();
    for (const QString& componentId : components) {
        ComponentInstanceV2* instance = d->manager->getComponent(componentId);
        if (instance && instance->componentObject) {
            IComponent* component = dynamic_cast<IComponent*>(instance->componentObject);
            if (component) {
                component->onThemeChanged(theme);
            }
        }
    }
}

void ComponentService::notifyLanguageChanged(int language)
{
    QStringList components = d->manager->getLoadedComponentIds();
    for (const QString& componentId : components) {
        ComponentInstanceV2* instance = d->manager->getComponent(componentId);
        if (instance && instance->componentObject) {
            IComponent* component = dynamic_cast<IComponent*>(instance->componentObject);
            if (component) {
                component->onLanguageChanged(language);
            }
        }
    }
    // 向QML引擎发送语言变化事件，触发所有qsTr()重新求值
    if (d->qmlEngine) {
        QCoreApplication::postEvent(d->qmlEngine, new QEvent(QEvent::LanguageChange));
    }
}

#include "ComponentService.moc"
