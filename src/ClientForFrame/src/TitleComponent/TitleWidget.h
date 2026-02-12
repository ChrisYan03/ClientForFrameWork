#ifndef TITLEWIDGET_H
#define TITLEWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

class TitleWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TitleWidget(QWidget *parent = nullptr);

private:
    void setupUI();

signals:
    void startButtonClicked();

private slots:
    void onStartButtonClicked();

private:
    QPushButton *m_startButton;
};

#endif // TITLEWIDGET_H