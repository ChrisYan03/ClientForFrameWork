#ifndef COMPONENTREGISTRY_H
#define COMPONENTREGISTRY_H

#include <QMap>
#include <QObject>
#include <QStringList>
#include <QUrl>

/**
 * @brief 组件清单注册：页 URL、图标路径、显示名称（由 ComponentService 与 QML 写入）。
 */
class ComponentRegistry : public QObject
{
    Q_OBJECT

public:
    explicit ComponentRegistry(QObject *parent = nullptr);

    QStringList loadedComponentIds() const { return m_componentPageUrls.keys(); }
    int componentCount() const { return m_componentPageUrls.size(); }
    bool hasComponentPage(const QString &appId) const;

    void registerComponentIcon(const QString &appId, const QString &iconPath);
    void registerComponentName(const QString &appId, const QString &name);
    void registerComponentPage(const QString &appId, const QUrl &pageUrl);

    QString getComponentIconUrl(const QString &appId) const;
    QString getComponentName(const QString &appId) const;
    QUrl getComponentPageUrl(const QString &appId) const;

signals:
    void loadedComponentsChanged();
    void componentCountChanged();

private:
    QMap<QString, QString> m_componentIconPaths;
    QMap<QString, QUrl> m_componentPageUrls;
    QMap<QString, QString> m_componentNames;
};

#endif // COMPONENTREGISTRY_H
