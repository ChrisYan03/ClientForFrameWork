#include "TitleWidget.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout> 
#include <QLabel>

TitleWidget::TitleWidget(BaseWidget *parent)
    : BaseWidget(parent)
{
    setupUI();
}

void TitleWidget::setupUI()
{
    setFixedHeight(48);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    // 创建开始按钮
    m_startButton = new QPushButton("开始", this);
    m_startButton->setToolTip("点击开始执行操作"); // 设置提示文本
    
    // 设置按钮样式
    m_startButton->setStyleSheet(
        "QPushButton {"
        "   font-family: 'Microsoft YaHei';"  // 微软雅黑字体
        "   font-size: 14px;"                 // 字体大小14
        "   border-radius: 10px;"              // 圆角效果
        "   background-color: #4CAF50;"      // 背景色
        "   color: white;"                     // 文字颜色
        "   padding: 6px;"                    // 内边距
        "   min-width: 80px;"                  // 最小宽度
        "   min-height: 30px;"                 // 最小高度
        "}"
        "QPushButton:hover {"
        "   background-color: #45a049;"        // 鼠标悬停时的背景色
        "}"
        "QPushButton:pressed {"
        "   background-color: #3d8b40;"        // 按下时的背景色
        "}"
    );
    
    // 创建停止按钮
    m_stopButton = new QPushButton("停止", this);
    m_stopButton->setToolTip("点击停止执行操作"); // 设置提示文本
    
    // 设置停止按钮样式
    m_stopButton->setStyleSheet(
        "QPushButton {"
        "   font-family: 'Microsoft YaHei';"  // 微软雅黑字体
        "   font-size: 14px;"                 // 字体大小14
        "   border-radius: 10px;"              // 圆角效果
        "   background-color: #f44336;"      // 背景色（红色）
        "   color: white;"                     // 文字颜色
        "   padding: 6px;"                    // 内边距
        "   min-width: 80px;"                  // 最小宽度
        "   min-height: 30px;"                 // 最小高度
        "}"
        "QPushButton:hover {"
        "   background-color: #d32f2f;"        // 鼠标悬停时的背景色
        "}"
        "QPushButton:pressed {"
        "   background-color: #b71c1c;"        // 按下时的背景色
        "}"
    );
    // 连接信号槽
    connect(m_startButton, &QPushButton::clicked, this, &TitleWidget::onStartButtonClicked);
    connect(m_stopButton, &QPushButton::clicked, this, &TitleWidget::onStopButtonClicked);
    
    // 添加控件到布局
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();  // 添加弹性空间
    buttonLayout->addWidget(m_startButton);
    buttonLayout->addWidget(m_stopButton);
    layout->addLayout(buttonLayout);
    buttonLayout->addStretch();  // 添加弹性空间
    layout->addLayout(buttonLayout);
    setLayout(layout);
}

void TitleWidget::onStartButtonClicked()
{
    emit startButtonClicked(); // 发射开始信号
}

void TitleWidget::onStopButtonClicked()
{
    emit stopButtonClicked(); // 发射停止信号
}