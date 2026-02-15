#include "PicMatchWidget.h"
#include "PicPlayerApi.h"
#include "PicRecognitionApi.h"
#include "StbImage/stb_image.h"
#include <QHBoxLayout>
#include "LogUtil.h"
#include <QCoreApplication>
#include <QDir>

PicMatchWidget::PicMatchWidget(QWidget *parent)
    : QWidget(parent)
    , m_handle(-1)
    , m_playerWidget(nullptr)
{
    setStyleSheet("background-color: #ccffcc;");
}

PicMatchWidget::~PicMatchWidget()
{
    LOG_DEBUG("~PicMatchWidget");
    DestroyFaceRecognition();
    PicPlayer_UnInit();
    LOG_DEBUG("~PicMatchWidget suc");
}

void PicMatchWidget::InitUI()
{
    LOG_DEBUG("InitUI size: {} x {}", width(), height());
    QHBoxLayout* picMatchLayout = new QHBoxLayout(this);
    picMatchLayout->setSpacing(8);
    picMatchLayout->setContentsMargins(0, 0, 0, 0);
    m_playerWidget = new QWidget();
    QWidget * rightWidget = new QWidget();
    picMatchLayout->addWidget(m_playerWidget, 4);
    picMatchLayout->addWidget(rightWidget, 1);
    this->adjustSize();  // 确保布局生效
    // 获取当前运行路径
    QString currentPath = QDir(QCoreApplication::applicationDirPath()).absolutePath();
    InitFaceRecognition(currentPath.toLocal8Bit().constData());
}

void PicMatchWidget::InitPicPlayer()
{
    if (nullptr == m_playerWidget) {
        LOG_ERROR("m_playerWidget is nullptr");
        return;
    }
    PicPlayer_Init();
    m_handle = PicPlayer_CreateInstance();
    if (-1 == m_handle) {
        LOG_ERROR("PicPlayer_CreateInstance is failed!!");
        return;
    }
    PicPlayer_RegisterCallback(m_handle, (PlayerMsgCallback)PicCallbackByPlayer, this);
    PicPlayer_RegisterWindow(m_handle, static_cast<Window_ShowID>(m_playerWidget->winId()));
    PicPlayer_Play(m_handle);
}

void PicMatchWidget::Run()
{    
    InitPicPlayer();
    PicShowInfo* demodata = new PicShowInfo(); // 自带析构函数
    std::string imagename = "beauty_20250216152514";
    #ifdef __APPLE__
    const char* imagePath = "/Users/chrisyan/ClientForFrameWork/beauty_20250216152514.jpg";
#else
    const char* imagePath = "E:/ClientForFrameWork/beauty_20250216152514.jpg";
#endif
    std::memcpy(demodata->imageId, imagename.c_str(), imagename.size());
    demodata->picReadTime = 1;
    LoadJpegToRGBA(imagePath, demodata);
    FaceDetectionResult* faceResult = new FaceDetectionResult();
    std::memcpy(faceResult->imageId, imagename.c_str(), imagename.size());
    DetectFacesInRgba(demodata, faceResult);
    LOG_DEBUG("InitPicPlayer size: {} x {}", m_playerWidget->width(), m_playerWidget->height());
    PicPlayer_InputPicData(m_handle, 1, (void*)demodata);
    if (faceResult->faces != nullptr && faceResult->faceCount > 0) {
        for (int i = 0; i < faceResult->faceCount; ++i) {
            // 镜像处理：x坐标从 x 变为 (1.0 - x - width)，保持人脸宽度不变
            // 这样可以确保人脸在镜像后仍占据相同宽度，但位置相对于中心对称
            faceResult->faces[i].x = 1.0f - faceResult->faces[i].x - faceResult->faces[i].width;
        }
    }
    PicPlayer_InputFaceRecogResult(m_handle, (void*)faceResult);
}

void PicMatchWidget::Quit()
{
    if(m_handle != -1) {
        PicPlayer_DestroyInstance(m_handle);
        m_handle = -1;  // 重置句柄
    }
}

void PicMatchWidget::resizeEvent(QResizeEvent *event)
{
    if (nullptr == m_playerWidget) {
        QWidget::resizeEvent(event);
        return;
    }
    m_playerWidget->resize(width(), height());
    QWidget::resizeEvent(event);
}

void PicMatchWidget::Run(int )
{
    LOG_DEBUG("Run PicPlayer size: {} x {}", m_playerWidget->width(), m_playerWidget->height());
    PicShowInfo* demodata = new PicShowInfo(); // 自带析构函数
    std::string imagename = "xiaoxiaoyan_1";
    #ifdef __APPLE__
    const char* imagePath = "/Users/chrisyan/ClientForFrameWork/xiaoxiaoyan_1.jpg";
#else
    const char* imagePath = "E:/ClientForFrameWork/xiaoxiaoyan_1.jpg";
#endif
    std::memcpy(demodata->imageId, imagename.c_str(), imagename.size());
    demodata->picReadTime = 1;
    LoadJpegToRGBA(imagePath, demodata);
    FaceDetectionResult* faceResult = new FaceDetectionResult();
    std::memcpy(faceResult->imageId, imagename.c_str(), imagename.size());
    DetectFacesInRgba(demodata, faceResult);
    LOG_DEBUG("faceResult faceCount: {}", faceResult->faceCount);
    PicPlayer_InputPicData(m_handle, 1, (void*)demodata);
    if (faceResult->faces != nullptr && faceResult->faceCount > 0) {
        for (int i = 0; i < faceResult->faceCount; ++i) {
            // 镜像处理：x坐标从 x 变为 (1.0 - x - width)，保持人脸宽度不变
            // 这样可以确保人脸在镜像后仍占据相同宽度，但位置相对于中心对称
            faceResult->faces[i].x = 1.0f - faceResult->faces[i].x - faceResult->faces[i].width;
        }
    }
    PicPlayer_InputFaceRecogResult(m_handle, (void*)faceResult);
}

void* PicMatchWidget::PicCallbackByPlayer(int handle, int iMsg, void* pData, void* pUser)
{
    if (nullptr != pData && nullptr != pUser) {
        PicMatchWidget* pThis = reinterpret_cast<PicMatchWidget*>(pUser);
        if (Callback_ShowPicId == iMsg) {
            std::string showid((const char*)pData);
            LOG_DEBUG("showid : {}", showid.data());
            LOG_DEBUG("Run PicPlayer size: {} x {}", pThis->m_playerWidget->width(), pThis->m_playerWidget->height());
            pThis->Run(1);
        }
    }
    return nullptr;
}

void PicMatchWidget::LoadJpegToRGBA(const char* imagePath, PicShowInfo *demodata)
{
    // 参数校验
    if (!imagePath || !demodata) {
        LOG_ERROR("Invalid input parameters.");
        return;
    }

    int channels, picWidth, picHeight;
    // 使用 stbi_load 加载图像，并确保输出为 RGBA 格式
    unsigned char* imageData = stbi_load(imagePath, &picWidth, &picHeight, &channels, 4);
    if (!imageData) {
        LOG_ERROR("Failed to load image: {}, Reason: {}", imagePath, (stbi_failure_reason() ? stbi_failure_reason() : "Unknown error"));
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
        LOG_ERROR("Failed to allocate memory for image data.");
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
