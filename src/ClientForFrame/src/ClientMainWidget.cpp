#include "ClientMainWidget.h"
#include "TitleComponent/TitleWidget.h"
#include <QVBoxLayout>
#include <iostream>

ClientMainWidget::ClientMainWidget(QWidget *parent)
    : QWidget(parent)
    , m_pPicMatchWidget(nullptr)
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
    mainLayout->setContentsMargins(0, 0, 0, 0);
    TitleWidget *titleWidget = new TitleWidget();
    mainLayout->addWidget(titleWidget);
    m_pPicMatchWidget = new PicMatchWidget(this);
    mainLayout->addWidget(m_pPicMatchWidget);

    connect(titleWidget, &TitleWidget::startButtonClicked, [this]() {
        // 处理开始按钮点击事件
        m_pPicMatchWidget->Run();
    });
    connect(titleWidget, &TitleWidget::stopButtonClicked, [this]() {
        // 处理停止按钮点击事件
        m_pPicMatchWidget->Quit();
    });
}

void ClientMainWidget::DemoInit()
{
    m_pPicMatchWidget->InitUI();
}

void ClientMainWidget::DemoQuit()
{
    m_pPicMatchWidget->Quit();
}
