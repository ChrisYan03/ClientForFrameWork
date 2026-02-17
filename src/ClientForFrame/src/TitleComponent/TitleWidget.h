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

private slots:
    void onStartButtonClicked();
    void onStopButtonClicked();

private:
    QPushButton *m_startButton;
    QPushButton *m_stopButton;
};

#endif // TITLEWIDGET_H