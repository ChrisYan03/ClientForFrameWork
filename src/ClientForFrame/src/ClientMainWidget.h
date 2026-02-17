#ifndef CLIENTMAINWIDGET_H
#define CLIENTMAINWIDGET_H

#include "Common/BaseWidget.h"
#include "PicMatchComponent/PicMatchWidget.h"

class ClientMainWidget : public BaseWidget
{
    Q_OBJECT

public:
    ClientMainWidget(BaseWidget *parent = nullptr);
    ~ClientMainWidget();

    void ClientMainInit();
    void ClientMainQuit();

private:
    void InitMainUI();

private:
    BaseWidget* m_titleWidget;
    PicMatchWidget* m_pPicMatchWidget;
};
#endif // CLIENTMAINWIDGET_H
