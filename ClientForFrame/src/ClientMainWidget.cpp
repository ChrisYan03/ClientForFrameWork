#include "ClientMainWidget.h"
#include <QVBoxLayout>
#include <iostream>

ClientMainWidget::ClientMainWidget(QWidget *parent)
    : QWidget(parent)
    , m_pPicMatchWidget(new PicMatchWidget(this))
{
    setMinimumSize(1200, 700);

    InitMainUI();
}

ClientMainWidget::~ClientMainWidget()
{
    std::cout << "~ClientMainWidget";
}

void ClientMainWidget::InitMainUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0,0);
    QWidget * m_titleWidget = new QWidget();
    m_titleWidget->setStyleSheet("background-color: #c68fdc;");
    m_titleWidget->setFixedHeight(48);
    mainLayout->addWidget(m_titleWidget);
    mainLayout->addWidget(m_pPicMatchWidget);
}

void ClientMainWidget::DemoInit()
{
    m_pPicMatchWidget->InitUI();
}

void ClientMainWidget::DemoRun()
{
    m_pPicMatchWidget->Run();
}

void ClientMainWidget::DemoQuit()
{
    m_pPicMatchWidget->Quit();
}
