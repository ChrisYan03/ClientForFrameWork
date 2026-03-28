#include "ComponentRegistry.h"
#include "LogUtil.h"
#include <QCoreApplication>
#include <QFile>
#include <QUrl>

ComponentRegistry::ComponentRegistry(QObject *parent)
    : QObject(parent)
{
}

bool ComponentRegistry::hasComponentPage(const QString &appId) const
{
    return m_componentPageUrls.contains(appId);
}

void ComponentRegistry::registerComponentIcon(const QString &appId, const QString &iconPath)
{
    if (!appId.isEmpty() && !iconPath.isEmpty()) {
        if (QFile::exists(iconPath)) {
            m_componentIconPaths.insert(appId, iconPath);
            LOG_INFO("ComponentRegistry: Registered icon for {} -> {}", appId.toStdString(), iconPath.toStdString());
        } else {
            LOG_WARN("ComponentRegistry: Icon file not found: {}", iconPath.toStdString());
            m_componentIconPaths.insert(appId, QString());
        }
    }
}

void ComponentRegistry::registerComponentName(const QString &appId, const QString &name)
{
    if (!appId.isEmpty() && !name.isEmpty()) {
        m_componentNames.insert(appId, name);
        LOG_INFO("ComponentRegistry: Registered name for {} -> {}", appId.toStdString(), name.toStdString());
    }
}

QString ComponentRegistry::getComponentIconUrl(const QString &appId) const
{
    QString path = m_componentIconPaths.value(appId, QString());
    return path.isEmpty() ? path : QUrl::fromLocalFile(path).toString();
}

QString ComponentRegistry::getComponentName(const QString &appId) const
{
    QString name = m_componentNames.value(appId, QString());
    if (name.isEmpty())
        return QString();
    return qApp->translate("PicMatchComponent", name.toUtf8().constData());
}

void ComponentRegistry::registerComponentPage(const QString &appId, const QUrl &pageUrl)
{
    if (!appId.isEmpty() && pageUrl.isValid()) {
        m_componentPageUrls.insert(appId, pageUrl);
        emit loadedComponentsChanged();
        emit componentCountChanged();
    }
}

QUrl ComponentRegistry::getComponentPageUrl(const QString &appId) const
{
    return m_componentPageUrls.value(appId, QUrl());
}
