#include "PlayerHostItem.h"
#include "../PicMatchComponent/PicMatchWidget.h"
#include "../Common/BaseWidget.h"
#include <QQuickWindow>
#include <QVBoxLayout>
#include <QWindow>
#include <QTimer>
#include <QtGlobal>
#include "LogUtil.h"

PlayerHostItem::PlayerHostItem(QQuickItem *parent)
    : QQuickItem(parent)
    , m_containerWidget(nullptr)
    , m_picMatchWidget(nullptr)
{
    setObjectName("playerHost");
    setFlag(ItemHasContents, false);
}

PlayerHostItem::~PlayerHostItem()
{
    if (m_containerWidget) {
        m_containerWidget->deleteLater();
        m_containerWidget = nullptr;
    }
    m_picMatchWidget = nullptr;
}

void PlayerHostItem::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickItem::geometryChange(newGeometry, oldGeometry);
    ensureWidgetCreated();
#if defined(Q_OS_WIN)
    if (m_containerWidget) {
        if (!m_geometryDeferTimer) {
            m_geometryDeferTimer = new QTimer(this);
            m_geometryDeferTimer->setSingleShot(true);
            connect(m_geometryDeferTimer, &QTimer::timeout, this, &PlayerHostItem::updateEmbeddedGeometry);
        }
        m_geometryDeferTimer->start(0);
        return;
    }
#endif
    updateEmbeddedGeometry();
}

void PlayerHostItem::ensureWidgetCreated()
{
    if (m_containerWidget || !window())
        return;

    QQuickWindow *quickWin = window();
    if (!quickWin)
        return;
#if defined(Q_OS_WIN)
    // Windows 下需等主窗口显示且布局完成后再创建嵌入窗口，否则嵌入位置/尺寸异常导致不显示
    if (!quickWin->isVisible() || width() < 10 || height() < 10) {
        QTimer::singleShot(50, this, [this]() { ensureWidgetCreated(); updateEmbeddedGeometry(); });
        return;
    }
#endif

    BaseWidget *container = new BaseWidget(nullptr);
    m_containerWidget = container;
    QVBoxLayout *layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    m_picMatchWidget = new PicMatchWidget(container);
    layout->addWidget(m_picMatchWidget);

    // 先给容器设好尺寸再 InitUI，保证左侧播放器与右侧人脸列布局比例正确（尤其 Windows）
    const int w = qRound(width());
    const int h = qRound(height());
    if (w > 0 && h > 0)
        m_containerWidget->setFixedSize(w, h);

    m_picMatchWidget->InitUI();

    (void)m_containerWidget->winId();
    QWindow *embedWin = m_containerWidget->windowHandle();
    if (embedWin) {
        embedWin->setParent(quickWin);
        embedWin->show();
    }

    updateEmbeddedGeometry();
#if defined(Q_OS_WIN)
    QTimer::singleShot(0, this, [this]() { updateEmbeddedGeometry(); });
#endif
    LOG_INFO("PlayerHostItem: embedded PicMatchWidget in QML window");
    emit widgetReady();
}

void PlayerHostItem::updateEmbeddedGeometry()
{
    if (!m_containerWidget || !m_containerWidget->windowHandle() || !window())
        return;

    const qreal w = width();
    const qreal h = height();
    if (w < 1 || h < 1)
        return;

    QPointF scenePos = mapToScene(QPointF(0, 0));
    int x = qRound(scenePos.x());
    int y = qRound(scenePos.y());
#if defined(Q_OS_WIN)
    // 防止嵌入窗口盖住标题栏：若布局未完成导致 y==0，则至少下移标题栏高度(与 MainWindow titleBarHeight 一致)
    const int titleBarHeight = 38;
    if (y < titleBarHeight)
        y = titleBarHeight;
#endif
    QRect geom(x, y, qRound(w), qRound(h));
    m_containerWidget->windowHandle()->setGeometry(geom);
    // 所有平台都同步设置容器尺寸，否则最大化时 QWidget 不随宿主 resize，嵌入的 GLFW 区域也不会更新
    m_containerWidget->setFixedSize(geom.width(), geom.height());
    if (!m_containerWidget->windowHandle()->isVisible())
        m_containerWidget->windowHandle()->show();
}
