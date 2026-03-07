#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <QObject>
#include <QVariantMap>

/**
 * @brief 纯 QML 框架的桥接类，暴露主题/状态及组件宿主注册。
 * 组件通过 registerComponentHost 注册，框架通过 invokeMethod 调用组件的 run/quit。
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

public:
    explicit AppController(QObject *parent = nullptr);
    ~AppController();

    QString statusText() const { return m_statusText; }
    QString pageTitle() const { return m_pageTitle; }
    bool hasRunnableComponent() const { return m_hasRunnableComponent; }
    bool isRunning() const { return m_isRunning; }
    int theme() const { return m_theme; }
    QVariantMap themeColors() const { return m_themeColors; }

    Q_INVOKABLE void setTheme(int theme);
    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void closeApp();

    /** 组件页加载时调用，注册宿主（QObject 需提供 run/quit 等 Q_INVOKABLE） */
    Q_INVOKABLE void registerComponentHost(QObject *hostItem);
    /** 组件页卸载时调用 */
    Q_INVOKABLE void unregisterComponentHost();
    /** 设置当前标题栏标题（如进入组件页时设为组件名，返回桌面时恢复默认） */
    Q_INVOKABLE void setPageTitle(const QString &title);
    Q_INVOKABLE void requestBackToDesktop();

signals:
    void statusTextChanged();
    void pageTitleChanged();
    void hasRunnableComponentChanged();
    void isRunningChanged();
    void themeChanged();
    void themeColorsChanged();
    void requestQuit();
    void backToDesktopRequested();

private:
    void setStatusText(const QString &text);
    void setHasRunnableComponent(bool on);
    void setRunning(bool on);
    void loadThemeColors();
    void applyThemeToPicPlayer();

    QString m_statusText;
    QString m_pageTitle;
    bool m_hasRunnableComponent = false;
    bool m_isRunning = false;
    int m_theme = 0;
    QVariantMap m_themeColors;
    QObject *m_componentHost = nullptr;
};

#endif // APPCONTROLLER_H
