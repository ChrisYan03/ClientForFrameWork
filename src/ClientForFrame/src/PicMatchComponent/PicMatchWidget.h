#ifndef PICMATCHWIDGET_H
#define PICMATCHWIDGET_H

#include "PicPlayerDataDef.h"
#include "../Common/BaseWidget.h"

class PicMatchWidget : public BaseWidget
{
    Q_OBJECT

public:
    PicMatchWidget(BaseWidget *parent = nullptr);
    ~PicMatchWidget();

    void InitUI();
    void InitPicPlayer();

    void Run();
    void Run(const std::string& showid);

    void Quit();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    static void* PicCallbackByPlayer(int handle, int iMsg, void* pData, void* pUser);
    bool LoadJpegToRGBA(const char* imagePath, PicShowInfo* demodata);
    void RotateImage90Degrees(unsigned char* imageData, unsigned char*& rotatedData, int width, int height, int channels);
    std::string GetNextImageName(); // 获取下一个图片名称的函数
    
private:
    int m_handle;
    BaseWidget * m_playerWidget;
    std::string m_showId;
};
#endif // PICMATCHWIDGET_H
