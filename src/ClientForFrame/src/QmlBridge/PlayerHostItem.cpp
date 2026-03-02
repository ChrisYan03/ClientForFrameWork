#include "PlayerHostItem.h"
#include "../PicMatchComponent/PicMatchWidget.h"
#include "../Common/BaseWidget.h"
#include <QQuickWindow>
#include <QVBoxLayout>
#include <QWindow>
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
    updateEmbeddedGeometry();
}

void PlayerHostItem::ensureWidgetCreated()
{
    if (m_containerWidget || !window())
        return;

    QQuickWindow *quickWin = window();
    if (!quickWin)
        return;

    BaseWidget *container = new BaseWidget(nullptr);
    m_containerWidget = container;
    QVBoxLayout *layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    m_picMatchWidget = new PicMatchWidget(container);
    layout->addWidget(m_picMatchWidget);
    m_picMatchWidget->InitUI();

    (void)m_containerWidget->winId();
    QWindow *embedWin = m_containerWidget->windowHandle();
    if (embedWin) {
        embedWin->setParent(quickWin);  // QQuickWindow* 转为 QWindow*
        embedWin->show();
    }

    updateEmbeddedGeometry();
    LOG_INFO("PlayerHostItem: embedded PicMatchWidget in QML window");
}

void PlayerHostItem::updateEmbeddedGeometry()
{
    if (!m_containerWidget || !m_containerWidget->windowHandle() || !window())
        return;

    QPointF scenePos = mapToScene(QPointF(0, 0));
    QRect geom(static_cast<int>(scenePos.x()),
               static_cast<int>(scenePos.y()),
               static_cast<int>(width()),
               static_cast<int>(height()));
    if (geom.isValid() && geom.width() > 0 && geom.height() > 0)
        m_containerWidget->windowHandle()->setGeometry(geom);
}
