#ifndef PLAYERHOSTITEM_H
#define PLAYERHOSTITEM_H

#include <QQuickItem>

class PicMatchWidget;
class QWidget;

/**
 * 在 QML 中嵌入 PicMatchWidget（QWidget），用于在主窗口指定区域显示播放器与人脸区域。
 * 将宿主 QWidget 的 windowHandle() 设为 QQuickWindow 子窗口并同步几何。
 */
class PlayerHostItem : public QQuickItem
{
    Q_OBJECT

public:
    explicit PlayerHostItem(QQuickItem *parent = nullptr);
    ~PlayerHostItem();

    /** 供 main_qml 获取并交给 AppController，用于 Run/Quit */
    PicMatchWidget *picMatchWidget() const { return m_picMatchWidget; }

protected:
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;

private:
    void ensureWidgetCreated();
    void updateEmbeddedGeometry();

    QWidget *m_containerWidget;
    PicMatchWidget *m_picMatchWidget;
};

#endif // PLAYERHOSTITEM_H
