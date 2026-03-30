#ifndef IMAGEVIEWHOSTITEM_H
#define IMAGEVIEWHOSTITEM_H

#include <QQuickItem>
#include <QProcess>
#include <QPointer>
#include <QMap>
#include <map>

class QWindow;
class QWidget;
class QPushButton;

class ImageViewHostItem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    /// 与主框架 TranslationManager 一致：0=中文，1=英文；-1 表示未接入框架时用系统 QLocale
    Q_PROPERTY(int frameworkLanguage READ frameworkLanguage WRITE setFrameworkLanguage NOTIFY frameworkLanguageChanged)

public:
    enum class OverlayButtonType {
        MainToggle = 0,
        Like = 1,
        Close = 2
    };

    explicit ImageViewHostItem(QQuickItem* parent = nullptr);
    ~ImageViewHostItem() override;

    static void setComponentBasePath(const QString& basePath);

    bool running() const { return m_running; }
    int frameworkLanguage() const { return m_frameworkLanguage; }
    void setFrameworkLanguage(int lang);

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void toggle();
    /// 与框架 AppController::start/stop 通过 QMetaObject::invokeMethod 对接（同 start/stop）
    Q_INVOKABLE void run() { start(); }
    Q_INVOKABLE void quit() { stop(); }

signals:
    void runningChanged();
    void frameworkLanguageChanged();

protected:
    void geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) override;

private slots:
    void onWindowChanged(QQuickWindow* win);
    void syncOverlayGeometry();

private:
    void ensureHostWindowCreated();
    void updateHostWindowGeometry();
    void ensureButtonWidgetCreated();
    void updateButtonWidgetGeometry();
    void updateButtonStyleAndText();
    QString loadTextResource(const QString& qrcPath) const;
    void loadI18nAndThemeResources();
    QString overlayI18nJsonPath() const;
    QString trText(const QString& key, const QString& fallback) const;
    QString themeColor(const QString& key, const QString& fallback) const;
    QWidget* widgetForButton(OverlayButtonType type) const;
    QPushButton* buttonControl(OverlayButtonType type) const;
    void setButtonWidgetVisible(OverlayButtonType type, bool visible);
    void raiseButtonWidget(OverlayButtonType type);
    void onOverlayButtonClicked(OverlayButtonType type);
    bool isDarkMode() const;
    QString hostProgramPath() const;
    QStringList hostProgramArgs() const;

    static QString s_componentBasePath;

    QWindow* m_hostWindow = nullptr;
    std::map<OverlayButtonType, QPointer<QWidget>> m_buttonWidgets;
    std::map<OverlayButtonType, QPointer<QPushButton>> m_buttonControls;
    QPointer<QQuickWindow> m_boundWindow;
    QProcess* m_process = nullptr;
    QMap<QString, QString> m_i18n;
    QMap<QString, QString> m_theme;
    bool m_running = false;
    bool m_stopping = false;
    int m_frameworkLanguage = -1;
};

#endif // IMAGEVIEWHOSTITEM_H
