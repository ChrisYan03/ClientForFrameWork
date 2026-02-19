#include "TitleWidget.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFont>
#include <QApplication>
#include <QIcon>
#include <QEvent>
#include <QFile>
#include "LogUtil.h"
TitleWidget::TitleWidget(BaseWidget *parent)
    : BaseWidget(parent)
    , m_dragging(false)  // 初始化拖动状态
{
    setupUI();
}

// 添加鼠标事件处理实现
void TitleWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragPosition = event->globalPos() - parentWidget()->frameGeometry().topLeft();
        event->accept();
    }
}
void TitleWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        parentWidget()->move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}

void TitleWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = false;
        event->accept();
    }
}
bool TitleWidget::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::Enter) {
        if (watched == m_startButton && !m_startIconHover.isNull()) {
            m_startButton->setIcon(m_startIconHover);
            return false;
        }
        if (watched == m_stopButton && !m_stopIconHover.isNull()) {
            m_stopButton->setIcon(m_stopIconHover);
            return false;
        }
        if (watched == m_closeButton && !m_closeIconHover.isNull()) {
            m_closeButton->setIcon(m_closeIconHover);
            return false;
        }
    } else if (event->type() == QEvent::Leave) {
        if (watched == m_startButton && !m_startIcon.isNull()) {
            m_startButton->setIcon(m_startIcon);
            return false;
        }
        if (watched == m_stopButton && !m_stopIcon.isNull()) {
            m_stopButton->setIcon(m_stopIcon);
            return false;
        }
        if (watched == m_closeButton && !m_closeIcon.isNull()) {
            m_closeButton->setIcon(m_closeIcon);
            return false;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void TitleWidget::setupUI()
{
    setFixedHeight(40);  // VSCode风格的紧凑标题栏高度
    setStyleSheet(
        "TitleWidget {"
        "   background-color: #3c3c3c;"  // VSCode深色主题背景色
        "   border-bottom: 1px solid #252526;"  // 底部分割线
        "}"
    );

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(12, 0, 8, 0);  // VSCode风格的边距
    mainLayout->setSpacing(8);

    // 应用标题 - VSCode风格
    QLabel *titleLabel = new QLabel("图像识别系统");
    QFont titleFont("Segoe UI", 12);  // VSCode使用的字体
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet(
        "QLabel {"
        "   color: #CCCCCC;"  // VSCode标题文字颜色
        "   padding: 0px 8px;"
        "}"
    );
    
    mainLayout->addWidget(titleLabel);
    mainLayout->addStretch();

    // 添加状态指示器
    m_statusLabel = new QLabel("● 就绪");
    m_statusLabel->setFont(QFont("Segoe UI", 9));
    m_statusLabel->setStyleSheet(
        "QLabel {"
        "   color: #4EC9B0;"  // VSCode绿色状态指示器
        "   padding: 0px 8px;"
        "}"
    );
    mainLayout->addWidget(m_statusLabel);

    // 创建按钮（仅图标，VSCode风格）
    m_startButton = new QPushButton(this);
    m_startButton->setToolTip("开始执行图像匹配任务");
    m_startButton->setFixedSize(32, 24);  // VSCode风格的按钮尺寸

    m_stopButton = new QPushButton(this);
    m_stopButton->setToolTip("停止当前图像匹配任务");
    m_stopButton->setFixedSize(32, 24);

    m_closeButton = new QPushButton(this);
    m_closeButton->setToolTip("关闭应用程序");
    m_closeButton->setFixedSize(32, 24);

    // *** 添加按钮点击信号连接 ***
    connect(m_startButton, &QPushButton::clicked, this, &TitleWidget::onStartButtonClicked);
    connect(m_stopButton, &QPushButton::clicked, this, &TitleWidget::onStopButtonClicked);
    connect(m_closeButton, &QPushButton::clicked, this, &TitleWidget::onCloseButtonClicked);
    
    // 加载图标
    m_startIcon = QIcon(":/icons/start.svg");
    m_startIconHover = QIcon(":/icons/start_hover.svg");
    m_stopIcon = QIcon(":/icons/stop.svg");
    m_stopIconHover = QIcon(":/icons/stop_hover.svg");
    m_closeIcon = QIcon(":/icons/close.svg");
    m_closeIconHover = QIcon(":/icons/close_hover.svg");
    // 检查所有图标文件是否存在
    QFile startFile(":/icons/start.svg");
    LOG_INFO("Start icon file exists: {}", startFile.exists());
    QSize iconSize(16, 16);  // VSCode标准图标大小
    
    if (!m_startIcon.isNull()) {
        m_startButton->setIcon(m_startIcon);
        m_startButton->setIconSize(iconSize);
        m_startButton->installEventFilter(this);
    }
    if (!m_stopIcon.isNull()) {
        m_stopButton->setIcon(m_stopIcon);
        m_stopButton->setIconSize(iconSize);
        m_stopButton->installEventFilter(this);
    }
    if (!m_closeIcon.isNull()) {
        m_closeButton->setIcon(m_closeIcon);
        m_closeButton->setIconSize(iconSize);
        m_closeButton->installEventFilter(this);
    }

    // VSCode风格的按钮样式
    QString baseButtonStyle =
        "QPushButton {"
        "   background: transparent;"
        "   border: none;"
        "   border-radius: 4px;"
        "}"
        "QPushButton:hover {"
        "   background-color: rgba(90, 93, 94, 0.31);"  // VSCode悬停背景
        "}"
        "QPushButton:pressed {"
        "   background-color: rgba(90, 93, 94, 0.50);"  // VSCode按下背景
        "}";
        
    m_startButton->setStyleSheet(baseButtonStyle);
    m_stopButton->setStyleSheet(baseButtonStyle);
    m_closeButton->setStyleSheet(baseButtonStyle);
    
    // 添加按钮到布局
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(4);
    buttonLayout->addWidget(m_startButton);
    buttonLayout->addWidget(m_stopButton);
    buttonLayout->addWidget(m_closeButton);
    
    mainLayout->addLayout(buttonLayout);
}

void TitleWidget::onStartButtonClicked()
{
    m_statusLabel->setText("● 运行中");
    m_statusLabel->setStyleSheet(
        "QLabel {"
        "   color: #4EC9B0;"
        "   padding: 0px 8px;"
        "}"
    );
    emit startButtonClicked();
}

void TitleWidget::onStopButtonClicked()
{
    m_statusLabel->setText("● 已停止");
    m_statusLabel->setStyleSheet(
        "QLabel {"
        "   color: #DCDCAA;"
        "   padding: 0px 8px;"
        "}"
    );
    emit stopButtonClicked();
}

void TitleWidget::onCloseButtonClicked()
{
    m_statusLabel->setText("● 正在关闭");
    m_statusLabel->setStyleSheet(
        "QLabel {"
        "   color: #F48771;"
        "   padding: 0px 8px;"
        "}"
    );
    emit closeButtonClicked();
}