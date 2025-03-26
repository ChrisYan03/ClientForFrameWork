#ifndef PICMATCHWIDGET_H
#define PICMATCHWIDGET_H

#include "PicPlayerDataDef.h"
#include <QWidget>

class PicMatchWidget : public QWidget
{
    Q_OBJECT

public:
    PicMatchWidget(QWidget *parent = nullptr);
    ~PicMatchWidget();

    void InitUI();
    void InitPicPlayer(QWidget* playerWidget);

    void Run();

private:
    void LoadJpegToRGBA(const char* imagePath, PicShowInfo& demodata);

private:
    int m_handle;
};
#endif // PICMATCHWIDGET_H
