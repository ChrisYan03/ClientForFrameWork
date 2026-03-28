#include "ComponentTabManager.h"
#include "ComponentRegistry.h"

ComponentTabManager::ComponentTabManager(ComponentRegistry *registry, QObject *parent)
    : QObject(parent)
    , m_registry(registry)
{
}

QVariantList ComponentTabManager::componentTabs() const
{
    QVariantList tabs;
    if (!m_registry)
        return tabs;
    for (const QString &appId : m_openedTabs) {
        QVariantMap tab;
        tab[QStringLiteral("appId")] = appId;
        tab[QStringLiteral("name")] = m_registry->getComponentName(appId);
        tab[QStringLiteral("iconPath")] = m_registry->getComponentIconUrl(appId);
        tab[QStringLiteral("isActive")] = (appId == m_currentTabAppId);
        tab[QStringLiteral("isOpened")] = true;
        tabs.append(tab);
    }
    return tabs;
}

int ComponentTabManager::openComponentTab(const QString &appId)
{
    if (!m_registry || appId.isEmpty() || !m_registry->hasComponentPage(appId))
        return -1;

    if (m_openedTabs.contains(appId)) {
        switchToTab(appId);
        return m_openedTabs.indexOf(appId);
    }

    m_openedTabs.append(appId);
    m_currentTabAppId = appId;
    emit componentTabsChanged();
    emit currentTabChanged(appId);
    return m_openedTabs.size() - 1;
}

void ComponentTabManager::closeComponentTab(const QString &appId)
{
    int idx = m_openedTabs.indexOf(appId);
    if (idx < 0)
        return;

    bool wasActive = (appId == m_currentTabAppId);
    m_openedTabs.removeAt(idx);

    if (wasActive) {
        if (!m_openedTabs.isEmpty()) {
            int newIdx = qMin(idx, m_openedTabs.size() - 1);
            m_currentTabAppId = m_openedTabs[newIdx];
        } else {
            m_currentTabAppId.clear();
        }
        emit currentTabChanged(m_currentTabAppId);
    }
    emit componentTabsChanged();
}

void ComponentTabManager::switchToTab(const QString &appId)
{
    if (appId == m_currentTabAppId || !m_openedTabs.contains(appId))
        return;
    m_currentTabAppId = appId;
    emit componentTabsChanged();
    emit currentTabChanged(appId);
}

QString ComponentTabManager::currentTabAppId() const
{
    return m_currentTabAppId;
}
