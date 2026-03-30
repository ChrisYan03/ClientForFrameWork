/**
 * @file WebUrlComponentLoader.h
 * @brief Web URL组件加载器
 */
#ifndef WEB_URL_COMPONENT_LOADER_H
#define WEB_URL_COMPONENT_LOADER_H

#include "../../IComponentLoader.h"
#include <QString>

class WebUrlWrapper : public QObject
{
    Q_OBJECT
public:
    explicit WebUrlWrapper(const QString& url, QObject* parent = nullptr);
    ~WebUrlWrapper() override;

    bool load();
    QString getUrl() const { return m_url; }
    void setUrl(const QString& url);
    void* getCEFBrowserView() const { return m_cefBrowserView; }
    void setCEFBrowserView(void* view) { m_cefBrowserView = view; }

signals:
    void pageLoaded();
    void loadFailed(const QString& error);

private:
    QString m_url;
    void* m_cefBrowserView = nullptr;
};

class WebUrlComponentLoader : public QObject, public IComponentLoader
{
    Q_OBJECT
public:
    explicit WebUrlComponentLoader(QObject* parent = nullptr);
    ~WebUrlComponentLoader() override;

    QObject* load(const ComponentManifest& manifest, const QString& basePath) override;
    bool unload(QObject* component) override;
    bool canLoad(const ComponentManifest& manifest, const QString& basePath) const override;
    ComponentType supportedType() const override { return ComponentType_WebUrl; }

private:
    QString extractUrl(const ComponentManifest& manifest) const;
    bool isValidUrl(const QString& url) const;
    QMap<QString, WebUrlWrapper*> m_loadedWebComponents;
};

#endif // WEB_URL_COMPONENT_LOADER_H
