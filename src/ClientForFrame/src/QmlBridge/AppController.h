#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <QObject>
#include <QVariantMap>

class PicMatchWidget;
class QWidget;

class PlayerHostItem;

/**
 * @brief 供 QML 调用的 C++ 桥接类，暴露开始/停止/关闭及状态。
 * 主题色从 themes/light.json、themes/dark.json 读取，通过 themeColors 暴露给 QML，无需为每种颜色定义成员。
 */
class AppController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(bool hasRunnableComponent READ hasRunnableComponent NOTIFY hasRunnableComponentChanged)
    Q_PROPERTY(bool isRunning READ isRunning NOTIFY isRunningChanged)
    Q_PROPERTY(int theme READ theme NOTIFY themeChanged)
    Q_PROPERTY(QVariantMap themeColors READ themeColors NOTIFY themeColorsChanged)

public:
    explicit AppController(QObject *parent = nullptr);
    ~AppController();

    QString statusText() const { return m_statusText; }
    bool hasRunnableComponent() const { return m_hasRunnableComponent; }
    bool isRunning() const { return m_isRunning; }
    int theme() const { return m_theme; }
    QVariantMap themeColors() const { return m_themeColors; }

    /** 换肤：0 浅色 1 深色，由设置页调用 */
    Q_INVOKABLE void setTheme(int theme);

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void closeApp();

    Q_INVOKABLE void registerPicMatchHost(QObject *hostItem);
    Q_INVOKABLE void unregisterPicMatchHost();
    Q_INVOKABLE void requestBackToDesktop();

    void setPlayer(PicMatchWidget *widget, QWidget *hostWindow);

signals:
    void statusTextChanged();
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
    bool m_hasRunnableComponent = false;
    bool m_isRunning = false;
    int m_theme = 0;
    QVariantMap m_themeColors;
    QWidget *m_hostWindow = nullptr;
    PicMatchWidget *m_picMatchWidget = nullptr;
};

#endif // APPCONTROLLER_H
