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
    void Run(int );

private:
    static void* PicCallbackByPlayer(int handle, int iMsg, void* pData, void* pUser);
    void LoadJpegToRGBA(const char* imagePath, PicShowInfo* demodata);
    void RotateImage90Degrees(unsigned char* imageData, unsigned char*& rotatedData, int width, int height, int channels);

private:
    int m_handle;
};
#endif // PICMATCHWIDGET_H
