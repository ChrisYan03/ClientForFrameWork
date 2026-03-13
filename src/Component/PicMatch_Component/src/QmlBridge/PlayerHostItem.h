#ifndef PLAYERHOSTITEM_H
#define PLAYERHOSTITEM_H

#include <QQuickItem>
#include <QObject>
#include <QMetaObject>

class QWindow;
class QTimer;

/**
 * 纯 QQuickItem：用 QWindow 作为播放区宿主，将 OpenGL 播放器嵌入 QML，不再嵌入 PicMatchWidget。
 * 提供 hostWindow（QWindow*）供 ViewModel 注册；run/quit/applyTheme 转发给 viewModel。
 */
class PlayerHostItem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QObject* hostWindow READ hostWindow NOTIFY hostWindowChanged)
    Q_PROPERTY(QObject* viewModel READ viewModel WRITE setViewModel NOTIFY viewModelChanged)

public:
    explicit PlayerHostItem(QQuickItem *parent = nullptr);
    ~PlayerHostItem();

    QObject* hostWindow() const;

    Q_INVOKABLE void run();
    Q_INVOKABLE void quit();
    Q_INVOKABLE void applyTheme(QVariantMap themeColors);

    QObject* viewModel() const { return m_viewModel; }
    void setViewModel(QObject* vm);

signals:
    void widgetReady();
    void hostWindowChanged();
    void viewModelChanged();

protected:
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;

private:
    void ensureHostWindowCreated();
    void updateHostWindowGeometry();
    void notifyPlayerWindowSize();
    void onViewModelRunningChanged(bool running);

    QWindow* m_hostWindow = nullptr;
    QObject* m_viewModel = nullptr;
    QMetaObject::Connection m_debugRunningConn;
#if defined(Q_OS_WIN)
    QTimer* m_geometryDeferTimer = nullptr;
#endif
};

#endif // PLAYERHOSTITEM_H
