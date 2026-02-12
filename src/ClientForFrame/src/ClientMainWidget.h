#ifndef CLIENTMAINWIDGET_H
#define CLIENTMAINWIDGET_H

#include <QWidget>
#include "PicMatchComponent/PicMatchWidget.h"

class ClientMainWidget : public QWidget
{
    Q_OBJECT

public:
    ClientMainWidget(QWidget *parent = nullptr);
    ~ClientMainWidget();

    void DemoInit();
    void DemoQuit();

private:
    void InitMainUI();

private:
    QWidget* m_titleWidget;
    PicMatchWidget* m_pPicMatchWidget;
};
#endif // CLIENTMAINWIDGET_H
