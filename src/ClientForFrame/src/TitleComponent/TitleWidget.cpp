#include "TitleWidget.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout> 
#include <QLabel>
#include <QFont>
#include <QApplication>

TitleWidget::TitleWidget(BaseWidget *parent)
    : BaseWidget(parent)
{
    setupUI();
}

void TitleWidget::setupUI()
{
    setFixedHeight(70);
    setStyleSheet(
        "TitleWidget {"
        "   background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #4a6fa5, stop: 1 #3a5a8a);"
        "   border-bottom: 1px solid #2c3e50;"
        "}"
    );

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 5, 10, 5);

    QHBoxLayout* topLayout = new QHBoxLayout();
    
    // 应用标题
    QLabel *titleLabel = new QLabel("图像识别与匹配系统");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet(
        "QLabel {"
        "   color: white;"
        "   padding: 5px;"
        "}"
    );
    
    topLayout->addWidget(titleLabel);
    topLayout->addStretch();
    
    // 创建按钮
    m_startButton = new QPushButton("开始", this);
    m_startButton->setToolTip("开始执行图像匹配任务");
    
    m_stopButton = new QPushButton("停止", this);
    m_stopButton->setToolTip("停止当前图像匹配任务");

    m_closeButton = new QPushButton("关闭", this);
    m_closeButton->setToolTip("关闭应用程序");
    
    // 设置按钮样式
    QString buttonStyle = 
        "QPushButton {"
        "   font-family: 'Microsoft YaHei';"
        "   font-size: 14px;"
        "   font-weight: bold;"
        "   border-radius: 6px;"
        "   padding: 8px 16px;"
        "   min-width: 80px;"
        "   min-height: 30px;"
        "}"
        "QPushButton:hover {"
        "   border: 2px solid rgba(255, 255, 255, 0.5);"
        "}"
        "QPushButton:pressed {"
        "   border: 2px solid rgba(0, 0, 0, 0.2);"
        "}";
        
    m_startButton->setStyleSheet(
        buttonStyle +
        "QPushButton {"
        "   background-color: #27ae60;"
        "   color: white;"
        "}"
        "QPushButton:hover {"
        "   background-color: #2ecc71;"
        "}"
    );
    
    m_stopButton->setStyleSheet(
        buttonStyle +
        "QPushButton {"
        "   background-color: #e7e43c;"
        "   color: white;"
        "}"
        "QPushButton:hover {"
        "   background-color: #c0be2b;"
        "}"
    );

    m_closeButton->setStyleSheet(
        buttonStyle +
        "QPushButton {"
        "   background-color: #e74c3c;"
        "   color: white;"
        "}"
        "QPushButton:hover {"
        "   background-color: #c0392b;"
        "}"
    );
    
    // 连接信号槽
    connect(m_startButton, &QPushButton::clicked, this, &TitleWidget::onStartButtonClicked);
    connect(m_stopButton, &QPushButton::clicked, this, &TitleWidget::onStopButtonClicked);
    connect(m_closeButton, &QPushButton::clicked, this, &TitleWidget::onCloseButtonClicked);
    
    // 添加按钮到布局
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_startButton);
    buttonLayout->addWidget(m_stopButton);
    buttonLayout->addWidget(m_closeButton);
    
    topLayout->addLayout(buttonLayout);
    
    layout->addLayout(topLayout);
    
    // 添加状态栏区域
    QHBoxLayout* statusLayout = new QHBoxLayout();
    m_statusLabel = new QLabel("就绪");
    m_statusLabel->setStyleSheet(
        "QLabel {"
        "   color: #ecf0f1;"
        "   font-size: 12px;"
        "   padding: 2px;"
        "}"
    );
    statusLayout->addWidget(m_statusLabel);
    statusLayout->addStretch();
    
    layout->addLayout(statusLayout);
}

void TitleWidget::onStartButtonClicked()
{
    m_statusLabel->setText("正在运行...");
    emit startButtonClicked();
}

void TitleWidget::onStopButtonClicked()
{
    m_statusLabel->setText("已停止");
    emit stopButtonClicked();
}

void TitleWidget::onCloseButtonClicked()
{
    m_statusLabel->setText("正在关闭...");
    emit closeButtonClicked();
}