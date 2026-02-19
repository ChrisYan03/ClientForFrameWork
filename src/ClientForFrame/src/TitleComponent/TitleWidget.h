#ifndef TITLEWIDGET_H
#define TITLEWIDGET_H

#include "../Common/BaseWidget.h"
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QIcon>
#include <QPoint>
#include <QMouseEvent>

class TitleWidget : public BaseWidget
{
    Q_OBJECT

public:
    explicit TitleWidget(BaseWidget *parent = nullptr);
    bool eventFilter(QObject* watched, QEvent* event) override;

protected:
    // 添加鼠标事件处理以支持窗口拖动
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void setupUI();

signals:
    void startButtonClicked();
    void stopButtonClicked();
    void closeButtonClicked();

private slots:
    void onStartButtonClicked();
    void onStopButtonClicked();
    void onCloseButtonClicked();

private:
    QPushButton *m_startButton;
    QPushButton *m_stopButton;
    QPushButton *m_closeButton;
    QLabel* m_statusLabel;
    QIcon m_startIcon, m_startIconHover;
    QIcon m_stopIcon, m_stopIconHover;
    QIcon m_closeIcon, m_closeIconHover;
    
    // 拖动相关的成员变量
    bool m_dragging;
    QPoint m_dragPosition;
};

#endif // TITLEWIDGET_H