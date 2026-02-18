#ifndef PICMATCHWIDGET_H
#define PICMATCHWIDGET_H

#include "PicPlayerDataDef.h"
#include "../Common/BaseWidget.h"
#include "FaceShowWidget.h"

class PicMatchWidget : public BaseWidget
{
    Q_OBJECT

public:
    PicMatchWidget(BaseWidget *parent = nullptr);
    ~PicMatchWidget();

    void InitUI();
    void InitPicPlayer();

    void Run();
    void Quit();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    static void* PicCallbackByPlayer(int handle, int iMsg, void* pData, void* pUser);
    void OnRun(const std::string& showid);
    void UpdatePic(const std::string& showid, const std::string& imagePath);
    bool LoadJpegToRGBA(const char* imagePath, PicShowInfo* demodata);
    void RotateImage90Degrees(unsigned char* imageData, unsigned char*& rotatedData, int width, int height, int channels);
    std::string GetNextImageName(); // 获取下一个图片名称的函数
    
private:
    int m_handle;
    BaseWidget * m_playerWidget;
    FaceShowWidget* m_faceShowWidget;
    std::string m_showId;
    std::vector<std::string> m_imageNames;      // Store image names as member
    size_t m_currentIndex;                      // Current index as member
    bool m_initialized;                         // Initialization flag as member
};
#endif // PICMATCHWIDGET_H
