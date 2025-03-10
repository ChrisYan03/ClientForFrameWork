#include "ClientMainWidget.h"
#include "PicPlayerApi.h"
#include <QLabel>

ClientMainWidget::ClientMainWidget(QWidget *parent)
    : QWidget(parent)
{
    PicPlayer_Init();
    PicPlayer_Play(1);
    QLabel *qlab = new QLabel(this);
    qlab->setText("player....");
}

ClientMainWidget::~ClientMainWidget()
{

}
