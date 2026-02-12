#include "TitleWidget.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

TitleWidget::TitleWidget(QWidget *parent)
    : QWidget(parent)
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
    
    // 连接信号槽
    connect(m_startButton, &QPushButton::clicked, this, &TitleWidget::onStartButtonClicked);
    
    // 添加控件到布局
    layout->addWidget(m_startButton, 0, Qt::AlignCenter);
    
    setLayout(layout);
}

void TitleWidget::onStartButtonClicked()
{
    emit startButtonClicked(); // 发射信号
}