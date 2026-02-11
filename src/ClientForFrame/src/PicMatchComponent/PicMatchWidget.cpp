#include "PicMatchWidget.h"
#include "PicPlayerApi.h"
#include "StbImage/stb_image.h"
#include <QHBoxLayout>
#include <QDebug>
#include <iostream>

PicMatchWidget::PicMatchWidget(QWidget *parent)
    : QWidget(parent)
    , m_handle(-1)
{
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
    PicPlayer_RegisterCallback(m_handle, (PlayerMsgCallback)PicCallbackByPlayer, this);
    PicPlayer_RegisterWindow(m_handle, static_cast<Window_ShowID>(playerWidget->winId()));
    PicPlayer_Play(m_handle);
}

void PicMatchWidget::Run()
{
    PicShowInfo* demodata = new PicShowInfo(); // 自带析构函数
    std::string imagename = "xiaoxiaoyan_2";
    #ifdef __APPLE__
    const char* imagePath = "/Users/chrisyan/ClientForFrameWork/xiaoxiaoyan_2.jpg";
#else
    const char* imagePath = "E:/ClientForFrameWork/xiaoxiaoyan_2.jpg";
#endif
    std::memcpy(demodata->imageId, imagename.c_str(), imagename.size());
    demodata->picReadTime = 1;
    LoadJpegToRGBA(imagePath, demodata);
    PicPlayer_InputPicData(m_handle, 1, (void*)demodata);
}

void PicMatchWidget::Quit()
{
    if(m_handle != -1)
        PicPlayer_DestroyInstance(m_handle);
}

void PicMatchWidget::Run(int )
{
    PicShowInfo* demodata = new PicShowInfo(); // 自带析构函数
    std::string imagename = "mmexport1739695411204";
    const char* imagePath = "/Users/chrisyan/ClientForFrameWork/mmexport1739695411204.jpg";
    std::memcpy(demodata->imageId, imagename.c_str(), imagename.size());
    demodata->picReadTime = 1;
    LoadJpegToRGBA(imagePath, demodata);
    PicPlayer_InputPicData(m_handle, 1, (void*)demodata);
}

void* PicMatchWidget::PicCallbackByPlayer(int handle, int iMsg, void* pData, void* pUser)
{
    if (nullptr != pData && nullptr != pUser) {
        PicMatchWidget* pThis = reinterpret_cast<PicMatchWidget*>(pUser);
        if (Callback_ShowPicId == iMsg) {
            std::string showid((const char*)pData);
            qDebug()<< "showid : " << showid.data();
            //pThis->Run(1);
        }
    }
    return nullptr;
}

void PicMatchWidget::LoadJpegToRGBA(const char* imagePath, PicShowInfo *demodata)
{
    // 参数校验
    if (!imagePath || !demodata) {
        std::cerr << "Invalid input parameters." << std::endl;
        return;
    }

    int channels, picWidth, picHeight;
    // 使用 stbi_load 加载图像，并确保输出为 RGBA 格式
    unsigned char* imageData = stbi_load(imagePath, &picWidth, &picHeight, &channels, 4);
    if (!imageData) {
        
        std::cerr << "Failed to load image: " << imagePath << ". Reason: " 
                  << (stbi_failure_reason() ? stbi_failure_reason() : "Unknown error") << std::endl;
        return;
    }

   // 手动管理 stbi_load 分配的内存，使用 stbi_image_free 释放
   struct StbiDeleter {
        void operator()(unsigned char* ptr) { stbi_image_free(ptr); }
    };
    std::unique_ptr<unsigned char, StbiDeleter> imageDataPtr(imageData);

    // 假设图像需要旋转90度
    unsigned char* rotatedData = nullptr;
    RotateImage90Degrees(imageDataPtr.get(), rotatedData, picWidth, picHeight, 4);

    // 使用智能指针管理旋转后的数据
    std::unique_ptr<unsigned char[]> rotatedDataPtr(rotatedData);
    // 更新 PicShowInfo 数据
    demodata->imageRgbaLen = picWidth * picHeight * 4;
    demodata->picWidth = picHeight;
    demodata->picHeight = picWidth;
    // 分配目标内存并复制数据
    auto rgbaDataPtr = std::make_unique<char[]>(demodata->imageRgbaLen);
    if (!rgbaDataPtr) {
        std::cerr << "Failed to allocate memory for image data." << std::endl;
        return;
    }

   // 将图像数据复制到目标缓冲区
   std::memcpy(rgbaDataPtr.get(), rotatedData, demodata->imageRgbaLen);

   // 更新 demodata 指针
   demodata->imageRgbaData = rgbaDataPtr.release(); // 转移所有权给 demodata
}

void PicMatchWidget::RotateImage90Degrees(unsigned char* imageData, unsigned char*& rotatedData, int width, int height, int channels) {
    int newWidth = height;
    int newHeight = width;
    rotatedData = new unsigned char[newWidth * newHeight * channels];

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            for (int c = 0; c < channels; ++c) {
                rotatedData[((newHeight - 1 - x) * newWidth + y) * channels + c] = imageData[(y * width + x) * channels + c];
            }
        }
    }
}
