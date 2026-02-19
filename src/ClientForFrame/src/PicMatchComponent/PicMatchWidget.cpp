#include "PicMatchWidget.h"
#include "PicPlayerApi.h"
#include "PicRecognitionApi.h"
#include "StbImage/stb_image.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QScrollArea>
#include <QTabWidget>
#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QSlider>
#include <QComboBox>
#include "LogUtil.h"
#include <QCoreApplication>
#include <QDir>
#include <memory>

PicMatchWidget::PicMatchWidget(BaseWidget *parent)
    : BaseWidget(parent)
    , m_handle(-1)
    , m_playerWidget(nullptr)
    , m_faceShowWidget(nullptr)
    , m_showId("")
    , m_currentIndex(0)
    , m_initialized(false)
    , m_imageNames()
    , m_runing(false)
{
    setStyleSheet(
        "PicMatchWidget {"
        "   background-color: #faf9f8;"
        "   border: none;"
        "}"
    );
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
    // 创建主布局
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(2, 2, 2, 2); 
    mainLayout->setSpacing(2);

    // 初始化左侧控件 m_playerWidget（占 70% 宽度）
    if (!m_playerWidget) {
        m_playerWidget = new BaseWidget(this); // 假设 m_playerWidget 是 QWidget 类型
        m_playerWidget->setObjectName("PlayerWidget");
        m_playerWidget->setStyleSheet(
            "#PlayerWidget {"
            "    background-color: #E0F7FA;" // 浅蓝色背景
            "    border: 1px solid #B2EBF2;" // 边框颜色
            "    border-radius: 5px;"         // 圆角
            "}"
        );
        QSizePolicy playerPolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_playerWidget->setSizePolicy(playerPolicy);
    }

    // 初始化右侧控件 m_faceShowWidget（占 30% 宽度）
    if (!m_faceShowWidget) {
        m_faceShowWidget = new FaceShowWidget(this); // 假设 m_faceShowWidget 是 QWidget 类型
        m_faceShowWidget->setObjectName("FaceShowWidget");
        m_faceShowWidget->setStyleSheet(
            "#FaceShowWidget {"
            "    background-color: #E8F5E8;" // 浅绿色背景
            "    border: 1px solid #C8E6C9;" // 边框颜色
            "    border-radius: 5px;"         // 圆角
            "}"
        );
        QSizePolicy facePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_faceShowWidget->setSizePolicy(facePolicy);
    }

    // 将控件添加到布局中，并设置比例
    mainLayout->addWidget(m_playerWidget, 7);    // 左侧占比 7 份
    mainLayout->addWidget(m_faceShowWidget, 3);  // 右侧占比 3 份

    // 设置主布局
    setLayout(mainLayout);

    // 可选：在左右区域中添加具体内容
    // 左侧区域添加垂直布局
    QVBoxLayout* playerLayout = new QVBoxLayout(m_playerWidget);
    playerLayout->setContentsMargins(10, 10, 10, 10); // 内边距
    QLabel* playerLabel = new QLabel("播放区域", m_playerWidget);
    playerLabel->setAlignment(Qt::AlignCenter);
    playerLabel->setStyleSheet("font-size: 16px; color: #006064;");
    playerLayout->addWidget(playerLabel);

    // 右侧区域添加垂直布局
    QVBoxLayout* faceLayout = new QVBoxLayout(m_faceShowWidget);
    faceLayout->setContentsMargins(10, 10, 10, 10); // 内边距
    QLabel* faceLabel = new QLabel("人脸展示区域", m_faceShowWidget);
    faceLabel->setAlignment(Qt::AlignCenter);
    faceLabel->setStyleSheet("font-size: 16px; color: #2E7D32;");
    faceLayout->addWidget(faceLabel);

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
    if (!m_runing) {
        m_runing = true;
        InitPicPlayer();

        std::string imagename = "beauty_20250216152514";
    #ifdef __APPLE__
        const char* imagePath = "/Users/chrisyan/ClientForFrameWork/beauty_20250216152514.jpg";
    #else
        const char* imagePath = "E:/ClientForFrameWork/beauty_20250216152514.jpg";
    #endif
        UpdatePic(imagename, imagePath);
    }
}

void PicMatchWidget::Quit()
{
    // 先清理 UI，再销毁 PicPlayer，避免销毁窗口后触发的底层事件影响仍在执行的 UI 操作
    if (m_faceShowWidget) {
        m_faceShowWidget->clearFaceImages(true);
    }
    if(m_handle != -1) {
        PicPlayer_DestroyInstance(m_handle);
        m_handle = -1;
    }
    m_showId.clear();
    // 重置图片索引和初始化状态
    m_currentIndex = 0;
    m_initialized = false;
    m_runing = false;
    m_imageNames.clear();
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

void PicMatchWidget::OnRun(const std::string& showid)
{
    if (m_showId != showid) {
        m_showId = showid;
        m_faceShowWidget->clearFaceImages();
        m_faceShowWidget->triggerDisplay();
    }
    else {
        LOG_DEBUG("m_showId == showid; Run PicPlayer size: {} x {}", m_playerWidget->width(), m_playerWidget->height());
        return;
    }
    // todo：这里读取图片数据
    std::string imagename = GetNextImageName();
    LOG_DEBUG("Run PicPlayer GetNextImageName {}", imagename.c_str());
    
    if (imagename.empty() || imagename == "default_image") {
        LOG_DEBUG("GetNextImageName returned empty or default image name, skipping update");
        return;
    }
#ifdef __APPLE__
    std::string imagePath = "/Users/chrisyan/ClientForFrameWork/picdata/" + imagename + ".jpg";
#else
    std::string imagePath = "E:/ClientForFrameWork/picdata/"+ imagename + ".jpg";
#endif

    UpdatePic(imagename, imagePath);
}

void PicMatchWidget::UpdatePic(const std::string& showid, const std::string& imagePath)
{
    LOG_DEBUG("Run PicPlayer size: {} x {}", m_playerWidget->width(), m_playerWidget->height());
    std::unique_ptr<PicShowInfo> demodata = std::make_unique<PicShowInfo>();
    std::unique_ptr<FaceDetectionResult> faceResult = std::make_unique<FaceDetectionResult>();

    std::strncpy(demodata->imageId, showid.c_str(), IMAGE_ID_LEN - 1);
    demodata->imageId[IMAGE_ID_LEN - 1] = '\0';
    demodata->picReadTime = 1;
    // 同样处理faceResult的imageId
    std::strncpy(faceResult->imageId, showid.c_str(), IMAGE_ID_LEN - 1);
    faceResult->imageId[IMAGE_ID_LEN - 1] = '\0';
    
    // 加载图片数据
    if (!LoadJpegToRGBA(imagePath.c_str(), demodata.get())) {
        LOG_ERROR("Failed to load image: {}", imagePath);
        return;
    }
    LOG_DEBUG("InitPicPlayer size: {} x {}", m_playerWidget->width(), m_playerWidget->height());
    // 1、输入图片数据到播放器
    if (!PicPlayer_InputPicData(m_handle, 1, (void*)demodata.get())) {
        LOG_ERROR("Failed to input picture data to player");
        return;
    }

    // 2、执行人脸检测
    if (DetectFacesInRgba(demodata.get(), faceResult.get()) != 0) {
        LOG_ERROR("Face detection failed");
        return;
    }
    
    // 3、镜像处理人脸坐标
    if (faceResult->faces != nullptr && faceResult->faceCount > 0) {
        for (int i = 0; i < faceResult->faceCount; ++i) {
            // 镜像处理：x坐标从 x 变为 (1.0 - x - width)
            faceResult->faces[i].x = 1.0f - faceResult->faces[i].x - faceResult->faces[i].width;
        }
    }
    
    // 4、输入人脸识别结果到播放器
    if (!PicPlayer_InputFaceRecogResult(m_handle, (void*)faceResult.get())) {
        LOG_ERROR("Failed to input face recognition result to player");
    }
    
    // 5、显示图片中的人脸信息
    if (faceResult->faces != nullptr && faceResult->faceCount > 0) {
        for (int i = 0; i < faceResult->faceCount; ++i) {
            m_faceShowWidget->addFaceImages(faceResult->faces[i].faceImageData, faceResult->faces[i].faceImageLength, faceResult->faces[i].faceImageWidth, faceResult->faces[i].faceImageHeight);
        }
    }
    // 智能指针会在作用域结束时自动释放内存
}

std::string PicMatchWidget::GetNextImageName()
{
    #ifdef __APPLE__
    const std::string folderPath = "/Users/chrisyan/ClientForFrameWork/picdata/";
#else
    const std::string folderPath = "E:/ClientForFrameWork/picdata/";
#endif
    
     // 第一次调用时初始化
    if (!m_initialized) {
        try {
            QDir directory(QString::fromStdString(folderPath));
            if (directory.exists()) {
                directory.setFilter(QDir::Files);
                directory.setNameFilters(QStringList() << "*.jpg" << "*.jpeg" << "*.png" << "*.bmp");
                directory.setSorting(QDir::Name);
                
                QFileInfoList fileList = directory.entryInfoList();
                
                for (const QFileInfo& fileInfo : fileList) {
                    std::string baseName = fileInfo.baseName().toStdString();
                    m_imageNames.push_back(baseName);
                }
                
                LOG_INFO("Initialized image list with {} images", m_imageNames.size());
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Error initializing image list: {}", e.what());
        }
        m_initialized = true;
    }
    
     // 返回下一个图片名
    if (!m_imageNames.empty() && m_currentIndex < m_imageNames.size()) {
        std::string imageName = m_imageNames[m_currentIndex];
        m_currentIndex++; // 不再循环，只递增
        return imageName;
    }
    else if (!m_imageNames.empty() && m_currentIndex >= m_imageNames.size()) {
        // 所有图片都已遍历完，返回空字符串
        return "";
    }
    
    // 如果没有找到图片，返回默认值
    return "default_image";
}


void* PicMatchWidget::PicCallbackByPlayer(int handle, int iMsg, void* pData, void* pUser)
{
    if (nullptr != pData && nullptr != pUser) {
        PicMatchWidget* pThis = reinterpret_cast<PicMatchWidget*>(pUser);
        if (Callback_ShowPicId == iMsg) {
            std::string showid((const char*)pData);
            LOG_DEBUG("showid : {}", showid.data());
            LOG_DEBUG("Run PicPlayer size: {} x {}", pThis->m_playerWidget->width(), pThis->m_playerWidget->height());
            QMetaObject::invokeMethod(pThis, [pThis, showid]() {
                pThis->OnRun(showid);
            }, Qt::QueuedConnection);
        }
    }
    return nullptr;
}

bool PicMatchWidget::LoadJpegToRGBA(const char* imagePath, PicShowInfo *demodata)
{
    // 参数校验
    if (!imagePath || !demodata) {
        LOG_ERROR("Invalid input parameters.");
        return false;
    }

    int channels, picWidth, picHeight;
    // 使用 stbi_load 加载图像，并确保输出为 RGBA 格式
    unsigned char* imageData = stbi_load(imagePath, &picWidth, &picHeight, &channels, 4);
    if (!imageData) {
        LOG_ERROR("Failed to load image: {}, Reason: {}", imagePath, (stbi_failure_reason() ? stbi_failure_reason() : "Unknown error"));
        return false;
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
        return false;
    }

   // 将图像数据复制到目标缓冲区
   std::memcpy(rgbaDataPtr.get(), rotatedData, demodata->imageRgbaLen);

   // 更新 demodata 指针
   demodata->imageRgbaData = rgbaDataPtr.release(); // 转移所有权给 demodata
   return true;
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
