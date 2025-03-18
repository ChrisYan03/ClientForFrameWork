#include "ClientMainWidget.h"
#include <iostream>

ClientMainWidget::ClientMainWidget(QWidget *parent)
    : QWidget(parent)
    , m_pPicMatchWidget(new PicMatchWidget(this))
{
    m_pPicMatchWidget->InitUI();
}

ClientMainWidget::~ClientMainWidget()
{
    delete m_pPicMatchWidget; // 手动释放内存
    std::cout << "~ClientMainWidget";
}
