#ifndef COMPONENTTABMANAGER_H
#define COMPONENTTABMANAGER_H

#include <QObject>
#include <QStringList>
#include <QVariantList>

class ComponentRegistry;

/**
 * @brief 多组件标签页：打开顺序、当前激活项（依赖 ComponentRegistry 取名称与图标）。
 */
class ComponentTabManager : public QObject
{
    Q_OBJECT

public:
    explicit ComponentTabManager(ComponentRegistry *registry, QObject *parent = nullptr);

    QVariantList componentTabs() const;

    int openComponentTab(const QString &appId);
    void closeComponentTab(const QString &appId);
    void switchToTab(const QString &appId);
    QString currentTabAppId() const;

signals:
    void componentTabsChanged();
    void currentTabChanged(const QString &appId);

private:
    ComponentRegistry *m_registry = nullptr;
    QStringList m_openedTabs;
    QString m_currentTabAppId;
};

#endif // COMPONENTTABMANAGER_H
