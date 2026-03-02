#include "AppController.h"
#include "../PicMatchComponent/PicMatchWidget.h"

AppController::AppController(QObject *parent)
    : QObject(parent)
    , m_hostWindow(nullptr)
    , m_picMatchWidget(nullptr)
{
    setStatusText(QStringLiteral("● 就绪"));
}

AppController::~AppController()
{
    if (m_hostWindow) {
        m_hostWindow->deleteLater();
        m_hostWindow = nullptr;
    }
    m_picMatchWidget = nullptr;
}

void AppController::setStatusText(const QString &text)
{
    if (m_statusText != text) {
        m_statusText = text;
        emit statusTextChanged();
    }
}

void AppController::setPlayer(PicMatchWidget *widget, QWidget *hostWindow)
{
    m_picMatchWidget = widget;
    m_hostWindow = hostWindow;
}

void AppController::start()
{
    setStatusText(QStringLiteral("● 运行中"));
    if (m_hostWindow) {
        m_hostWindow->show();
        m_hostWindow->raise();
    }
    if (m_picMatchWidget)
        m_picMatchWidget->Run();
}

void AppController::stop()
{
    setStatusText(QStringLiteral("● 已停止"));
    if (m_picMatchWidget)
        m_picMatchWidget->Quit();
    if (m_hostWindow)
        m_hostWindow->hide();
}

void AppController::closeApp()
{
    if (m_picMatchWidget)
        m_picMatchWidget->Quit();
    emit requestQuit();
}
