#include "ClientMainWidget.h"
#include "TitleComponent/TitleWidget.h"
#include <QVBoxLayout>
#include "LogUtil.h"

ClientMainWidget::ClientMainWidget(QWidget *parent)
    : QWidget(parent)
    , m_pPicMatchWidget(nullptr)
{
    setMinimumSize(1400, 900);
    InitMainUI();
}

ClientMainWidget::~ClientMainWidget()
{
    LOG_DEBUG("~ClientMainWidget");
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

void ClientMainWidget::ClientMainInit()
{
    m_pPicMatchWidget->InitUI();
}

void ClientMainWidget::ClientMainQuit()
{
    m_pPicMatchWidget->Quit();
}
