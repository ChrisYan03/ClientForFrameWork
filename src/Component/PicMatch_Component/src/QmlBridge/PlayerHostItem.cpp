#include "PlayerHostItem.h"
#include "PicPlayerApi.h"
#include "PicMatchViewModel.h"
#include <QQuickWindow>
#include <QWindow>
#include <QVariantMap>
#include <QTimer>
#include <QMetaObject>
#include <QtGlobal>
#if defined(Q_OS_WIN)
#include <windows.h>
#endif
#include "LogUtil.h"

namespace {
bool IsViewModelRunning(QObject* vm)
{
    return vm ? vm->property("running").toBool() : false;
}
}

PlayerHostItem::PlayerHostItem(QQuickItem *parent)
    : QQuickItem(parent)
{
    setObjectName("playerHost");
    setFlag(ItemHasContents, false);
}

PlayerHostItem::~PlayerHostItem()
{
    if (m_debugRunningConn) {
        QObject::disconnect(m_debugRunningConn);
    }
    if (m_hostWindow) {
        m_hostWindow->destroy();
        m_hostWindow->deleteLater();
        m_hostWindow = nullptr;
    }
    m_viewModel = nullptr;
}

void PlayerHostItem::setViewModel(QObject* vm)
{
    if (m_viewModel == vm)
        return;

    if (m_debugRunningConn) {
        QObject::disconnect(m_debugRunningConn);
        m_debugRunningConn = QMetaObject::Connection();
    }

    m_viewModel = vm;
    if (m_viewModel) {
        if (auto* typedVm = qobject_cast<PicMatchViewModel*>(m_viewModel)) {
            m_debugRunningConn = QObject::connect(
                typedVm, &PicMatchViewModel::runningChanged, this,
                [this](bool running) {
                    onViewModelRunningChanged(running);
                },
                Qt::QueuedConnection);
        }
    }
    emit viewModelChanged();
}

QObject* PlayerHostItem::hostWindow() const
{
    return m_hostWindow;
}

void PlayerHostItem::run()
{
    if (m_viewModel) {
        QMetaObject::invokeMethod(m_viewModel, "run", Qt::QueuedConnection);
    }
}

void PlayerHostItem::quit()
{
    if (m_viewModel) {
        QMetaObject::invokeMethod(m_viewModel, "stop", Qt::QueuedConnection);
    }
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
    if (m_viewModel) {
        QMetaObject::invokeMethod(m_viewModel, "applyTheme", Qt::QueuedConnection,
            Q_ARG(QVariantMap, themeColors));
    }
}

void PlayerHostItem::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickItem::geometryChange(newGeometry, oldGeometry);
    ensureHostWindowCreated();
#if defined(Q_OS_WIN)
    if (m_hostWindow) {
        if (!m_geometryDeferTimer) {
            m_geometryDeferTimer = new QTimer(this);
            m_geometryDeferTimer->setSingleShot(true);
            connect(m_geometryDeferTimer, &QTimer::timeout, this, [this](){
                updateHostWindowGeometry();
                notifyPlayerWindowSize();
            });
        }
        m_geometryDeferTimer->start(0);
        return;
    }
#endif
    updateHostWindowGeometry();
    notifyPlayerWindowSize();
}

void PlayerHostItem::ensureHostWindowCreated()
{
    if (m_hostWindow || !window())
        return;

    QQuickWindow* quickWin = window();
    if (!quickWin)
        return;
#if defined(Q_OS_WIN)
    if (!quickWin->isVisible() || width() < 10 || height() < 10) {
        QTimer::singleShot(50, this, [this]() {
            ensureHostWindowCreated();
            updateHostWindowGeometry();
            notifyPlayerWindowSize();
        });
        return;
    }
#endif

    m_hostWindow = new QWindow(quickWin);
    m_hostWindow->setFlags(Qt::Widget);

    const int w = qRound(width());
    const int h = qRound(height());
    if (w > 0 && h > 0)
        m_hostWindow->setGeometry(0, 0, w, h);

    m_hostWindow->setParent(quickWin);
    const bool vmRunning = IsViewModelRunning(m_viewModel);
    if (vmRunning) {
        m_hostWindow->show();
    } else {
        m_hostWindow->hide();
    }

    LOG_DEBUG("PlayerHostItem: host window created, running={}, visible={}",
              vmRunning, m_hostWindow->isVisible());

    updateHostWindowGeometry();
#if defined(Q_OS_WIN)
    HWND hwnd = reinterpret_cast<HWND>(m_hostWindow->winId());
    if (hwnd) {
        RedrawWindow(hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    }
#endif
    quickWin->update();
    quickWin->requestUpdate();
#if defined(Q_OS_WIN)
    QTimer::singleShot(0, this, [this]() { updateHostWindowGeometry(); notifyPlayerWindowSize(); });
#endif
    LOG_INFO("PlayerHostItem: created QWindow host for OpenGL player (no QWidget embed)");
    emit widgetReady();
    emit hostWindowChanged();
}

void PlayerHostItem::updateHostWindowGeometry()
{
    if (!m_hostWindow || !window())
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
    m_hostWindow->setGeometry(x, y, qRound(w), qRound(h));
    const bool vmRunning = IsViewModelRunning(m_viewModel);
    if (vmRunning && !m_hostWindow->isVisible())
        m_hostWindow->show();
    if (!vmRunning && m_hostWindow->isVisible())
        m_hostWindow->hide();

    notifyPlayerWindowSize();
}

void PlayerHostItem::notifyPlayerWindowSize()
{
    if (!m_viewModel || !m_hostWindow || width() < 1 || height() < 1)
        return;
    const int w = qRound(width());
    const int h = qRound(height());
    if (w > 0 && h > 0) {
        QMetaObject::invokeMethod(m_viewModel, "setPlayerWindowSize", Qt::QueuedConnection,
            Q_ARG(int, w), Q_ARG(int, h));
    }
}

void PlayerHostItem::onViewModelRunningChanged(bool running)
{
    ensureHostWindowCreated();
    if (!m_hostWindow)
        return;

    if (running) {
        if (!m_hostWindow->isVisible()) {
            m_hostWindow->show();
        }
        updateHostWindowGeometry();
        notifyPlayerWindowSize();
        if (QQuickWindow* quickWin = window()) {
            quickWin->update();
            quickWin->requestUpdate();
        }
    } else {
        m_hostWindow->hide();
        if (QQuickWindow* quickWin = window()) {
            quickWin->update();
            quickWin->requestUpdate();
        }
#if defined(Q_OS_WIN)
        HWND hwnd = reinterpret_cast<HWND>(m_hostWindow->winId());
        if (hwnd) {
            RedrawWindow(hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW | RDW_ALLCHILDREN);
        }
#endif
    }
}
