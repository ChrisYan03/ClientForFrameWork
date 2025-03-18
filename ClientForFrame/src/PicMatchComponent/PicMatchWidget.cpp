#include "PicMatchWidget.h"
#include "PicPlayerApi.h"
#include <QHBoxLayout>
#include <iostream>

PicMatchWidget::PicMatchWidget(QWidget *parent)
    : QWidget(parent)
    , m_handle(-1)
{
    setMinimumSize(1200, 800);
    setStyleSheet("background-color: #ccffcc;");
}

PicMatchWidget::~PicMatchWidget()
{
    std::cout << "~PicMatchWidget";
    PicPlayer_UnInit();
}

void PicMatchWidget::InitUI()
{
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(0, 0, 0,0);
    QWidget * playerWidget = new QWidget();
    QWidget * leftWidget = new QWidget();
    QWidget * rightWidget = new QWidget();
    mainLayout->addWidget(leftWidget, 1);
    mainLayout->addWidget(playerWidget, 3);
    mainLayout->addWidget(rightWidget, 1);
    InitPicPlayer(playerWidget);
}

void PicMatchWidget::InitPicPlayer(QWidget* playerWidget)
{
    if (nullptr == playerWidget) {
        std::cerr << "playerWidget is nullptr" << std::endl;
        return;
    }
    PicPlayer_Init();
    m_handle = PicPlayer_CreateInstance();
    if (-1 == m_handle) {
        std::cerr << "PicPlayer_CreateInstance is failed!!" << std::endl;
        return;
    }
    PicPlayer_RegisterWindow(m_handle, static_cast<Window_ShowID>(playerWidget->winId()));
    PicPlayer_Play(m_handle);
}
