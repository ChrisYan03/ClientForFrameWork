#ifndef PLAYERHOSTITEM_H
#define PLAYERHOSTITEM_H

#include <QQuickItem>
#include <QObject>

class PicMatchWidget;
class QTimer;
class QWidget;

/**
 * 在 QML 中嵌入 PicMatchWidget（QWidget），用于在主窗口指定区域显示播放器与人脸区域。
 * 向框架提供 run/quit 与 hostWindow，供 AppController 通过 QObject 接口调用。
 */
class PlayerHostItem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QObject* hostWindow READ hostWindow NOTIFY hostWindowChanged)

public:
    explicit PlayerHostItem(QQuickItem *parent = nullptr);
    ~PlayerHostItem();

    PicMatchWidget *picMatchWidget() const { return m_picMatchWidget; }
    QObject* hostWindow() const;

    Q_INVOKABLE void run();
    Q_INVOKABLE void quit();
    /** 由框架在主题变更时调用，将 contentBackground 应用到 PicPlayer */
    Q_INVOKABLE void applyTheme(QVariantMap themeColors);

signals:
    void widgetReady();
    void hostWindowChanged();

protected:
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;

private:
    void ensureWidgetCreated();
    void updateEmbeddedGeometry();

    QWidget *m_containerWidget;
    PicMatchWidget *m_picMatchWidget;
#if defined(Q_OS_WIN)
    QTimer *m_geometryDeferTimer = nullptr;
#endif
};

#endif // PLAYERHOSTITEM_H
