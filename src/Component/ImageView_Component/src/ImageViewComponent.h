#ifndef IMAGEVIEWCOMPONENT_H
#define IMAGEVIEWCOMPONENT_H

#include <QObject>
#include <QPointer>
#include <QQmlEngine>
#include "IComponent.h"

class ImageViewComponent : public QObject, public IComponent
{
    Q_OBJECT

public:
    explicit ImageViewComponent(QObject* parent = nullptr);
    ~ImageViewComponent() override;

    int initialize(void* engine, const char* basePath) override;
    void shutdown() override;
    void registerQmlTypes(void* engine) override;
    void* getInterface(const char* interfaceName) override;
    void onThemeChanged(int theme) override;
    void onLanguageChanged(int language) override;

    Q_INVOKABLE void setManifest(const ComponentManifest& manifest) { m_manifest = manifest; }

private:
    QPointer<QQmlEngine> m_engine;
    QString m_basePath;
    bool m_initialized = false;

public:
    ComponentManifest m_manifest;
};

#endif // IMAGEVIEWCOMPONENT_H
