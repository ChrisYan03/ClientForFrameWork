#include "PicMatchWidget.h"
#include "PicPlayerApi.h"
#include "StbImage/stb_image.h"
#include <QHBoxLayout>
#include <iostream>

PicMatchWidget::PicMatchWidget(QWidget *parent)
    : QWidget(parent)
    , m_handle(-1)
{
    setMinimumSize(1200, 652);
    setStyleSheet("background-color: #ccffcc;");
}

PicMatchWidget::~PicMatchWidget()
{
    std::cout << "~PicMatchWidget";
    PicPlayer_UnInit();
    std::cout << "~PicMatchWidget suc";
}

void PicMatchWidget::InitUI()
{
    QHBoxLayout* picMatchLayout = new QHBoxLayout(this);
    picMatchLayout->setSpacing(8);
    picMatchLayout->setContentsMargins(0, 0, 0,0);
    QWidget * playerWidget = new QWidget();
    QWidget * rightWidget = new QWidget();
    picMatchLayout->addWidget(playerWidget, 4);
    picMatchLayout->addWidget(rightWidget, 1);
    InitPicPlayer(playerWidget);
}

void PicMatchWidget::InitPicPlayer(QWidget* playerWidget)
{
    if (nullptr == playerWidget) {
        std::cerr << "playerWidget is nullptr" << std::endl;
        return;
    }
    PicPlayer_Init();
    m_handle = PicPlayer_CreateInstance();
    if (-1 == m_handle) {
        std::cerr << "PicPlayer_CreateInstance is failed!!" << std::endl;
        return;
    }
    PicPlayer_RegisterWindow(m_handle, static_cast<Window_ShowID>(playerWidget->winId()));
    PicPlayer_Play(m_handle);
}

void PicMatchWidget::Run()
{
    PicShowInfo demodata; // 自带析构函数
    std::string imagename = "beauty_20250216152514";
    const char* imagePath = "/Users/chrisyan/ClientForFrameWork/beauty_20250216152514.jpg";
    std::memcpy(demodata.imageId, imagename.c_str(), imagename.size());
    demodata.picReadTime = 1;
    LoadJpegToRGBA(imagePath, demodata);
    PicPlayer_InputPicData(m_handle, 1, (void*)&demodata);
}

void PicMatchWidget::LoadJpegToRGBA(const char* imagePath, PicShowInfo &demodata)
{
    int channels, picWidth, picHeight;
    // 使用 stbi_load 加载图像，并确保输出为 RGBA 格式
    unsigned char* imageData = stbi_load(imagePath, &picWidth, &picHeight, &channels, 4);
    if (!imageData) {
        std::cerr << "Failed to load image: " << imagePath << std::endl;
        return;
    }
    demodata.imageRgbaLen = (picWidth) * (picHeight) * 4;
    demodata.picWidth = picWidth;
    demodata.picHeight = picHeight;
    demodata.imageRgbaData = new char[demodata.imageRgbaLen];
    // 将图像数据复制到指定的 rgbaData 地址
    std::memcpy(demodata.imageRgbaData, imageData, demodata.imageRgbaLen);

    // 释放 stbi_load 分配的内存
    stbi_image_free(imageData);
}
