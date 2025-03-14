#include "ClientMainWidget.h"
#include "PicPlayerApi.h"
#include <QLabel>
#include <QWindow>

ClientMainWidget::ClientMainWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(800, 600);
    setStyleSheet("background-color: #ccffcc;");
    PicPlayer_Init();
    m_handle = PicPlayer_CreateInstance();
}

ClientMainWidget::~ClientMainWidget()
{

}

void ClientMainWidget::hhhhhh()
{
    PicPlayer_RegisterWindow(m_handle, static_cast<Window_ShowID>(this->winId()));
    PicPlayer_Play(m_handle);
}
