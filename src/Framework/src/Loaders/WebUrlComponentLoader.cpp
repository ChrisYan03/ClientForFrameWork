/**
 * @file WebUrlComponentLoader.cpp
 * @brief Web URL组件加载器实现
 */
#include "WebUrlComponentLoader.h"
#include <QUrl>
#include "LogUtil.h"

// ==================== WebUrlWrapper 实现 ====================

WebUrlWrapper::WebUrlWrapper(const QString& url, QObject* parent)
    : QObject(parent)
    , m_url(url)
{
}

WebUrlWrapper::~WebUrlWrapper()
{
    // CEF清理逻辑（具体实现取决于CEF集成）
}

bool WebUrlWrapper::load()
{
    // TODO: 实现CEF加载逻辑
    LOG_INFO("Loading web URL: {}", m_url.toStdString());
    emit pageLoaded();
    return true;
}

void WebUrlWrapper::setUrl(const QString& url)
{
    m_url = url;
}

// ==================== WebUrlComponentLoader 实现 ====================

WebUrlComponentLoader::WebUrlComponentLoader(QObject* parent)
    : QObject(parent)
{
}

WebUrlComponentLoader::~WebUrlComponentLoader()
{
    // 清理所有Web组件
    qDeleteAll(m_loadedWebComponents);
    m_loadedWebComponents.clear();
}

QObject* WebUrlComponentLoader::load(const ComponentManifest& manifest, const QString& basePath)
{
    QString url = extractUrl(manifest);
    if (url.isEmpty() || !isValidUrl(url)) {
        LOG_WARN("Invalid or missing URL in manifest: {}",
                QString::fromStdString(manifest.id).toStdString());
        return nullptr;
    }

    WebUrlWrapper* wrapper = new WebUrlWrapper(url, this);
    wrapper->load();

    QString componentId = QString::fromStdString(manifest.id);
    m_loadedWebComponents[componentId] = wrapper;

    return wrapper;
}

bool WebUrlComponentLoader::unload(QObject* component)
{
    WebUrlWrapper* wrapper = qobject_cast<WebUrlWrapper*>(component);
    if (!wrapper) {
        return false;
    }

    // 从管理器中移除
    for (auto it = m_loadedWebComponents.begin(); it != m_loadedWebComponents.end(); ++it) {
        if (it.value() == wrapper) {
            m_loadedWebComponents.erase(it);
            break;
        }
    }

    delete wrapper;
    return true;
}

bool WebUrlComponentLoader::canLoad(const ComponentManifest& manifest, const QString& basePath) const
{
    QString url = extractUrl(manifest);
    return !url.isEmpty() && isValidUrl(url);
}

QString WebUrlComponentLoader::extractUrl(const ComponentManifest& manifest) const
{
    // TODO: 从manifest的额外字段中提取URL
    // 当前ComponentManifest没有url字段，需要扩展
    // 这里返回空字符串作为占位
    return QString();
}

bool WebUrlComponentLoader::isValidUrl(const QString& url) const
{
    QUrl urlObj(url);
    return urlObj.isValid() && !urlObj.isEmpty();
}
