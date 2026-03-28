#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <QObject>
#include <QUrl>
#include <QVariantMap>

class AppLocaleController;
class AppThemeController;
class ComponentRegistry;
class ComponentTabManager;

/**
 * @brief 纯 QML 框架桥接门面：组合主题、组件清单、标签页与宿主运行态，对外保持单一 appController 上下文属性。
 */
class AppController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(QString pageTitle READ pageTitle NOTIFY pageTitleChanged)
    Q_PROPERTY(bool hasRunnableComponent READ hasRunnableComponent NOTIFY hasRunnableComponentChanged)
    Q_PROPERTY(bool isRunning READ isRunning NOTIFY isRunningChanged)
    Q_PROPERTY(int theme READ theme NOTIFY themeChanged)
    Q_PROPERTY(QVariantMap themeColors READ themeColors NOTIFY themeColorsChanged)
    Q_PROPERTY(QStringList loadedComponents READ loadedComponents NOTIFY loadedComponentsChanged)
    Q_PROPERTY(int currentLanguage READ currentLanguage NOTIFY currentLanguageChanged)
    Q_PROPERTY(QString currentLanguageName READ currentLanguageName NOTIFY currentLanguageChanged)
    Q_PROPERTY(int componentCount READ componentCount NOTIFY componentCountChanged)
    Q_PROPERTY(QVariantList componentTabs READ componentTabs NOTIFY componentTabsChanged)

public:
    explicit AppController(QObject *parent = nullptr);
    ~AppController();

    QString statusText() const { return m_statusText; }
    QString pageTitle() const { return m_pageTitle; }
    bool hasRunnableComponent() const { return m_hasRunnableComponent; }
    bool isRunning() const { return m_isRunning; }
    int theme() const;
    QVariantMap themeColors() const;
    QStringList loadedComponents() const;
    int currentLanguage() const;
    QString currentLanguageName() const;
    int componentCount() const;
    QVariantList componentTabs() const;

    Q_INVOKABLE void setTheme(int theme);
    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void closeApp();
    Q_INVOKABLE void setLanguage(int language);
    Q_INVOKABLE QString getLanguageName(int language) const;
    Q_INVOKABLE void retranslateUi();

    Q_INVOKABLE void registerComponentHost(QObject *hostItem);
    Q_INVOKABLE void unregisterComponentHost();
    Q_INVOKABLE void setPageTitle(const QString &title);
    Q_INVOKABLE void requestBackToDesktop();

    Q_INVOKABLE void registerComponentIcon(const QString &appId, const QString &iconPath);
    Q_INVOKABLE QString getComponentIconPath(const QString &appId) const;
    Q_INVOKABLE void registerComponentName(const QString &appId, const QString &name);
    Q_INVOKABLE QString getComponentName(const QString &appId) const;

    Q_INVOKABLE void registerComponentPage(const QString &appId, const QUrl &pageUrl);
    Q_INVOKABLE QUrl getComponentPageUrl(const QString &appId) const;

    Q_INVOKABLE void requestShowBubbleMessage(const QString &message);

    Q_INVOKABLE int openComponentTab(const QString &appId);
    Q_INVOKABLE void closeComponentTab(const QString &appId);
    Q_INVOKABLE void switchToTab(const QString &appId);
    Q_INVOKABLE QString currentTabAppId() const;

signals:
    void statusTextChanged();
    void pageTitleChanged();
    void hasRunnableComponentChanged();
    void isRunningChanged();
    void themeChanged(int theme);
    void themeColorsChanged();
    void loadedComponentsChanged();
    void requestQuit();
    void backToDesktopRequested();
    void showBubbleMessageRequested(const QString &message);
    void currentLanguageChanged();
    void componentCountChanged();
    void componentTabsChanged();
    void currentTabChanged(const QString &appId);

private:
    void setStatusText(const QString &text);
    void setHasRunnableComponent(bool on);
    void setRunning(bool on);
    void applyThemeToCurrentHost();
    void refreshFrameworkBindingsAfterLocaleChange();

    QString m_statusText;
    QString m_pageTitle;
    bool m_hasRunnableComponent = false;
    bool m_isRunning = false;
    QObject *m_componentHost = nullptr;

    ComponentRegistry *m_registry = nullptr;
    ComponentTabManager *m_tabs = nullptr;
    AppThemeController *m_theme = nullptr;
    AppLocaleController *m_locale = nullptr;
};

#endif // APPCONTROLLER_H
