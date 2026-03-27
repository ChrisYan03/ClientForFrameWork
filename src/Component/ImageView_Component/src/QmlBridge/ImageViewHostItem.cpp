#include "ImageViewHostItem.h"
#include <QQuickWindow>
#include <QWindow>
#include <QFileInfo>
#include <QTimer>
#include <QPointer>
#include "LogUtil.h"

QString ImageViewHostItem::s_componentBasePath;

ImageViewHostItem::ImageViewHostItem(QQuickItem* parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents, false);
}

ImageViewHostItem::~ImageViewHostItem()
{
    stop();
    if (m_hostWindow) {
        m_hostWindow->destroy();
        m_hostWindow->deleteLater();
        m_hostWindow = nullptr;
    }
}

void ImageViewHostItem::setComponentBasePath(const QString& basePath)
{
    s_componentBasePath = basePath;
}

void ImageViewHostItem::start()
{
    if (m_running) {
        return;
    }
    if (m_stopping || (m_process && m_process->state() != QProcess::NotRunning)) {
        return;
    }
    ensureHostWindowCreated();
    if (!m_hostWindow) {
        LOG_WARN("ImageViewHostItem: host window is null, cannot start process");
        return;
    }
    m_hostWindow->show();

    const QString program = hostProgramPath();
    if (!QFileInfo::exists(program)) {
        LOG_WARN("ImageViewHostItem: player host executable not found: {}", program.toStdString());
        return;
    }

    if (!m_process) {
        m_process = new QProcess(this);
        connect(m_process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
                [this](int, QProcess::ExitStatus) {
                    m_stopping = false;
                    m_running = false;
                    emit runningChanged();
                });
    }

    updateHostWindowGeometry();
    m_process->start(program, hostProgramArgs());
    if (!m_process->waitForStarted(3000)) {
        LOG_ERROR("ImageViewHostItem: failed to start process: {}", m_process->errorString().toStdString());
        return;
    }

    m_running = true;
    emit runningChanged();
}

void ImageViewHostItem::stop()
{
    if (m_hostWindow) {
        // 先隐藏原生宿主窗口，避免返回桌面时出现残留覆盖。
        m_hostWindow->hide();
    }

    if (!m_process || !m_running) {
        if (m_hostWindow) {
            m_hostWindow->hide();
        }
        m_stopping = false;
        return;
    }

    m_running = false;
    emit runningChanged();
    m_stopping = true;

    m_process->terminate();
    QPointer<QProcess> processGuard(m_process);
    QTimer::singleShot(1500, this, [processGuard]() {
        if (processGuard && processGuard->state() != QProcess::NotRunning) {
            processGuard->kill();
        }
    });
}

void ImageViewHostItem::toggle()
{
    if (m_running) {
        stop();
    } else {
        start();
    }
}

void ImageViewHostItem::geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry)
{
    QQuickItem::geometryChange(newGeometry, oldGeometry);
    if (m_hostWindow) {
        updateHostWindowGeometry();
    }
}

void ImageViewHostItem::ensureHostWindowCreated()
{
    if (m_hostWindow || !window()) {
        return;
    }
    m_hostWindow = new QWindow(window());
    m_hostWindow->setFlags(Qt::Widget);
    m_hostWindow->setParent(window());
    m_hostWindow->hide();
}

void ImageViewHostItem::updateHostWindowGeometry()
{
    if (!m_hostWindow || !window()) {
        return;
    }
    QPointF scenePos = mapToScene(QPointF(0, 0));
    m_hostWindow->setGeometry(qRound(scenePos.x()), qRound(scenePos.y()), qRound(width()), qRound(height()));
}

QString ImageViewHostItem::hostProgramPath() const
{
#if defined(Q_OS_WIN)
    return s_componentBasePath + "bin/ImageViewPlayerHost.exe";
#else
    return s_componentBasePath + "bin/ImageViewPlayerHost";
#endif
}

QStringList ImageViewHostItem::hostProgramArgs() const
{
    QStringList args;
#if defined(Q_OS_WIN)
    const quintptr wid = m_hostWindow ? static_cast<quintptr>(m_hostWindow->winId()) : 0;
    args << "--parent-hwnd" << QString::number(wid);
    args << "--width" << QString::number(qRound(width()));
    args << "--height" << QString::number(qRound(height()));
#endif
    return args;
}
