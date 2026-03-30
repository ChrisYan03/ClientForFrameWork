/**
 * @file WebUrlComponentLoader.cpp
 * @brief Web URL组件加载器实现
 */
#include "WebUrlComponentLoader.h"
#include <QUrl>
#include "LogUtil.h"

WebUrlWrapper::WebUrlWrapper(const QString& url, QObject* parent)
    : QObject(parent)
    , m_url(url)
{
}

WebUrlWrapper::~WebUrlWrapper()
{
}

bool WebUrlWrapper::load()
{
    LOG_INFO("Loading web URL: {}", m_url.toStdString());
    emit pageLoaded();
    return true;
}

void WebUrlWrapper::setUrl(const QString& url)
{
    m_url = url;
}

WebUrlComponentLoader::WebUrlComponentLoader(QObject* parent)
    : QObject(parent)
{
}

WebUrlComponentLoader::~WebUrlComponentLoader()
{
    qDeleteAll(m_loadedWebComponents);
    m_loadedWebComponents.clear();
}

QObject* WebUrlComponentLoader::load(const ComponentManifest& manifest, const QString&)
{
    QString url = extractUrl(manifest);
    if (url.isEmpty() || !isValidUrl(url)) {
        LOG_WARN("Invalid or missing URL in manifest: {}",
                QString::fromStdString(manifest.id).toStdString());
        return nullptr;
    }

    WebUrlWrapper* wrapper = new WebUrlWrapper(url, this);
    wrapper->load();
    m_loadedWebComponents[QString::fromStdString(manifest.id)] = wrapper;
    return wrapper;
}

bool WebUrlComponentLoader::unload(QObject* component)
{
    WebUrlWrapper* wrapper = qobject_cast<WebUrlWrapper*>(component);
    if (!wrapper)
        return false;
    for (auto it = m_loadedWebComponents.begin(); it != m_loadedWebComponents.end(); ++it) {
        if (it.value() == wrapper) {
            m_loadedWebComponents.erase(it);
            break;
        }
    }
    delete wrapper;
    return true;
}

bool WebUrlComponentLoader::canLoad(const ComponentManifest& manifest, const QString&) const
{
    QString url = extractUrl(manifest);
    return !url.isEmpty() && isValidUrl(url);
}

QString WebUrlComponentLoader::extractUrl(const ComponentManifest&) const
{
    return QString();
}

bool WebUrlComponentLoader::isValidUrl(const QString& url) const
{
    QUrl urlObj(url);
    return urlObj.isValid() && !urlObj.isEmpty();
}
