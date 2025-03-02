#include "ClientMainWidget.h"
#include "PicPlayerApi.h"
#include <QLabel>

ClientMainWidget::ClientMainWidget(QWidget *parent)
    : QWidget(parent)
{
    PicPlayerApi a;
    a.play();
    QLabel *qlab = new QLabel(this);
    qlab->setText("player....");
}

ClientMainWidget::~ClientMainWidget()
{

}
