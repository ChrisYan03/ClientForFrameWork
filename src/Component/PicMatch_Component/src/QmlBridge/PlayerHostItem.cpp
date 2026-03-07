#include "PlayerHostItem.h"
#include "PicMatchWidget.h"
#include "BaseWidget.h"
#include "PicPlayerApi.h"
#include <QQuickWindow>
#include <QVariantMap>
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

QObject* PlayerHostItem::hostWindow() const
{
    return m_containerWidget;
}

void PlayerHostItem::run()
{
    if (m_picMatchWidget)
        m_picMatchWidget->Run();
}

void PlayerHostItem::quit()
{
    if (m_picMatchWidget)
        m_picMatchWidget->Quit();
}

void PlayerHostItem::applyTheme(QVariantMap themeColors)
{
    QVariant v = themeColors.value(QStringLiteral("contentBackground"));
    QString hex = v.toString();
    float r = 0.15f, g = 0.15f, b = 0.15f;
    if (!hex.isEmpty() && (hex.size() == 7 || hex.size() == 4)) {
        QString h = hex.startsWith(QLatin1Char('#')) ? hex.mid(1) : hex;
        if (h.size() >= 6) {
            bool ok;
            r = h.mid(0, 2).toInt(&ok, 16) / 255.0f;
            if (ok) g = h.mid(2, 2).toInt(&ok, 16) / 255.0f;
            if (ok) b = h.mid(4, 2).toInt(&ok, 16) / 255.0f;
        }
    }
    PicPlayer_SetBackgroundColor(r, g, b);
    if (m_picMatchWidget)
        m_picMatchWidget->applyTheme(themeColors);
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
    emit hostWindowChanged();
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
    const int titleBarHeight = 38;
    if (y < titleBarHeight)
        y = titleBarHeight;
#endif
    QRect geom(x, y, qRound(w), qRound(h));
    m_containerWidget->windowHandle()->setGeometry(geom);
    m_containerWidget->setFixedSize(geom.width(), geom.height());
    if (!m_containerWidget->windowHandle()->isVisible())
        m_containerWidget->windowHandle()->show();
}
