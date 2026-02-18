#ifndef TITLEWIDGET_H
#define TITLEWIDGET_H

#include "../Common/BaseWidget.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

class TitleWidget : public BaseWidget
{
    Q_OBJECT

public:
    explicit TitleWidget(BaseWidget *parent = nullptr);

private:
    void setupUI();

signals:
    void startButtonClicked();
    void stopButtonClicked();
    void closeButtonClicked();  // Add signal for closing the application

private slots:
    void onStartButtonClicked();
    void onStopButtonClicked();
    void onCloseButtonClicked();  // Add slot for handling close button click

private:
    QPushButton *m_startButton;
    QPushButton *m_stopButton;
    QPushButton *m_closeButton;  // Add close button
    // 添加新的成员变量声明
    QLabel* m_statusLabel;
};

#endif // TITLEWIDGET_H