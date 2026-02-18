#include "ClientMainWidget.h"
#include "TitleComponent/TitleWidget.h"
#include <QVBoxLayout>
#include <QSplitter>
#include <QFrame>
#include <QApplication>
#include "LogUtil.h"

ClientMainWidget::ClientMainWidget(BaseWidget *parent)
    : BaseWidget(parent)
    , m_pPicMatchWidget(nullptr)
{
    setMinimumSize(1400, 900);
    setStyleSheet(
        "ClientMainWidget {"
        "   background-color: #f0f0f0;"
        "}"
    );
    setWindowFlags(Qt::FramelessWindowHint);
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

    // 标题栏
    TitleWidget *titleWidget = new TitleWidget();
    mainLayout->addWidget(titleWidget);

    // 中央内容区域
    QSplitter* centralSplitter = new QSplitter(Qt::Vertical);
    centralSplitter->setStyleSheet(
        "QSplitter::handle {"
        "   background-color: #dcdcdc;"
        "   border: 1px solid #bcbcbc;"
        "   border-radius: 2px;"
        "}"
        "QSplitter {"
        "   background-color: #ffffff;"
        "}"
    );

    // 图像匹配组件
    m_pPicMatchWidget = new PicMatchWidget(this);
    centralSplitter->addWidget(m_pPicMatchWidget);

    // 添加中央分割器到主布局
    mainLayout->addWidget(centralSplitter);

    connect(titleWidget, &TitleWidget::startButtonClicked, [this]() {
        m_pPicMatchWidget->Run();
    });
    connect(titleWidget, &TitleWidget::stopButtonClicked, [this]() {
        m_pPicMatchWidget->Quit();
    });
    connect(titleWidget, &TitleWidget::closeButtonClicked, [this]() {
        ClientMainQuit();
        LOG_INFO("-------------------------------Performing cleanup...");
        QApplication::quit();
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