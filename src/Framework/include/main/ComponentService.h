/**
 * @file ComponentService.h
 * @brief 主框架组件服务接口
 */
#ifndef FRAMEWORK_COMPONENT_SERVICE_H
#define FRAMEWORK_COMPONENT_SERVICE_H

#include "ComponentTypes.h"
#include <QObject>
#include <QString>
#include <QQmlEngine>

class ComponentManagerPrivate;

class ComponentService : public QObject
{
    Q_OBJECT

public:
    explicit ComponentService(QObject* parent = nullptr);
    ~ComponentService();

    void setBasePath(const QString& basePath);
    bool initialize(QQmlEngine* engine, QObject* appController);
    void shutdown();

    bool loadComponent(const QString& componentId);
    bool unloadComponent(const QString& componentId);
    QStringList loadedComponents() const;
    QStringList getComponentsByType(ComponentType type) const;

    void notifyThemeChanged(int theme);
    void notifyLanguageChanged(int language);

signals:
    void componentLoaded(const QString& componentId);
    void componentLoadFailed(const QString& componentId, const QString& error);
    void componentUnloaded(const QString& componentId);

private:
    class ComponentManagerPrivate;
    ComponentManagerPrivate* d;
};

#endif // FRAMEWORK_COMPONENT_SERVICE_H
