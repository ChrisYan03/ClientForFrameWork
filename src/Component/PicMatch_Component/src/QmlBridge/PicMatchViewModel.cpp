#include "PicMatchViewModel.h"
#include "PicPlayerApi.h"
#include "PicRecognitionApi.h"
#include "StbImage/stb_image.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QFileDialog>
#include <QGuiApplication>
#include <QWindow>
#include <QTimer>
#include <QSettings>
#include <cstring>
#include <cstdlib>
#include <memory>
#include <string>
#include "LogUtil.h"

// 辅助函数：获取基础目录
static QString getEffectiveBaseDir()
{
    QString exeDir = QDir(QCoreApplication::applicationDirPath()).absolutePath();
#if defined(Q_OS_MAC)
    if (exeDir.contains(QLatin1String(".app/Contents/MacOS"))) {
        QDir dir(exeDir);
        if (dir.cdUp() && dir.cdUp() && dir.cdUp())
            return dir.absolutePath();
    }
#endif
    return exeDir;
}

// 辅助函数：获取组件bin路径
static QString componentBinPath()
{
    QString baseDir = getEffectiveBaseDir();
    QString componentBin = baseDir + QStringLiteral("/Component/PicMatch/bin");
    if (QDir(componentBin).exists())
        return componentBin;
#if defined(Q_OS_MAC)
    QString releaseBin = baseDir + QStringLiteral("/Release/Component/PicMatch/bin");
    if (QDir(releaseBin).exists())
        return releaseBin;
#endif
    QString exeDir = QDir(QCoreApplication::applicationDirPath()).absolutePath();
    QString fallback = exeDir + QStringLiteral("/Component/PicMatch/bin");
    if (QDir(fallback).exists())
        return fallback;
    return exeDir;
}

// 辅助函数：获取默认数据路径
static QString getDefaultDataPath()
{
#if defined(Q_OS_MAC)
    static const QString macDefaultPicdata(QStringLiteral("/Users/chrisyan/ClientForFrameWork/picdata"));
    QDir d(macDefaultPicdata);
    if (!d.exists())
        d.mkpath(QStringLiteral("."));
    return d.absolutePath();
#else
    return componentBinPath();
#endif
}

// 辅助函数：主题色获取
static QString themeColor(const QVariantMap& m, const char* key, const char* defaultHex)
{
    QString v = m.value(QLatin1String(key)).toString();
    return v.isEmpty() ? QLatin1String(defaultHex) : v;
}

// 静态回调中转（data 为临时指针，QueuedConnection 延后执行时已失效，Callback_ShowPicId 时必须先复制再入队）
void PICPLAYER_CALL picCallbackBridge(int handle, int msg, void* data, void* user)
{
    LOG_DEBUG("picCallbackBridge: handle={}, msg={}, user={}", handle, msg, user);
    if (!user)
        return;
    PicMatchViewModel* pThis = reinterpret_cast<PicMatchViewModel*>(user);
    if (msg == Callback_ShowPicId && data) {
        std::string showIdCopy(reinterpret_cast<const char*>(data));
        QMetaObject::invokeMethod(pThis, [pThis, handle, msg, showIdCopy]() {
            pThis->onPlayerCallback(handle, msg, static_cast<void*>(const_cast<char*>(showIdCopy.c_str())));
        }, Qt::QueuedConnection);
    } else {
        QMetaObject::invokeMethod(pThis, [pThis, handle, msg, data]() {
            pThis->onPlayerCallback(handle, msg, data);
        }, Qt::QueuedConnection);
    }
}

PicMatchViewModel::PicMatchViewModel(QObject* parent)
    : QObject(parent)
    , m_model(new PicMatchModel(this))
    , m_running(false)
    , m_statusText("")
    , m_playerHandle(-1)
#if defined(Q_OS_WIN)
    , m_resizeNotifyTimer(nullptr)
#endif
{
}

PicMatchViewModel::~PicMatchViewModel()
{
    shutdown();
}

void PicMatchViewModel::initialize()
{
    // 初始化数据路径
    QString dataPath = m_model->getCurrentDataPath();
    if (dataPath.isEmpty()) {
        dataPath = getDefaultDataPath();
        m_model->setDataPath(dataPath);
    }
    m_model->initializeImageList(dataPath);

    // 初始化人脸识别
    initFaceRecognition(componentBinPath());
}

void PicMatchViewModel::shutdown()
{
    stop();
    destroyFaceRecognition();
}

void PicMatchViewModel::run()
{
    if (!m_running) {
        m_running = true;
        initPicPlayer();

        // 先发出runningChanged信号，让PicMatchWidget同步playerHandle
        updateUIState();
        emit runningChanged(true);

        // 然后再处理第一张图片
        m_model->resetIndex();
        QString firstName = m_model->getNextImageName();
        if (!firstName.isEmpty() && firstName != "default_image") {
            const QString basePath = m_model->getCurrentDataPath().isEmpty() ?
                getDefaultDataPath() : m_model->getCurrentDataPath();
            const QString imagePath = basePath + QChar('/') + firstName;
            if (QFileInfo::exists(imagePath)) {
                const QString showId = QFileInfo(firstName).baseName();
                onImageUpdated(showId, imagePath);
            }
        }
    }
}

void PicMatchViewModel::stop()
{
    if (!m_running)
        return;

    m_running = false;

    // 清理人脸
    m_model->clearFaces();
    emit faceListChanged();

    // 销毁播放器
    if (m_playerHandle != -1) {
        PicPlayer_DestroyInstance(m_playerHandle);
        m_playerHandle = -1;
        emit handleChanged(-1);
    }
    PicPlayer_UnInit();

    m_model->setCurrentShowId("");
    m_model->resetIndex();

    updateUIState();
    emit runningChanged(false);
}

void PicMatchViewModel::loadConfig()
{
    // 从设置加载（Model构造函数已处理）
    emit dataPathChanged(m_model->getCurrentDataPath());
}

void PicMatchViewModel::saveConfig()
{
    // Model会自动保存
    m_model->setDataPath(m_model->getCurrentDataPath());
    emit configApplied();
}

void PicMatchViewModel::browseDataPath()
{
    // 这个方法供QML调用获取选择的路径
    // 实际路径设置在setDataPath中处理
}

void PicMatchViewModel::setDataPath(const QString& path)
{
    m_model->setDataPath(path);
    emit dataPathChanged(path);
}

void PicMatchViewModel::applyTheme(const QVariantMap& themeColors)
{
    m_lastThemeColors = themeColors.isEmpty() ? m_themeColors : themeColors;
    // 转发给PlayerHostItem处理
    // TODO: 通过信号通知UI层应用主题
}

void PicMatchViewModel::registerWindow(void* windowId)
{
    if (m_playerHandle < 0 || !windowId) {
        return;
    }

    // 注册窗口到PicPlayer
    PicPlayer_RegisterWindow(m_playerHandle, reinterpret_cast<Window_ShowID>(windowId));

    LOG_INFO("ViewModel: window registered, handle={}", m_playerHandle);

    // 通知窗口大小
#if defined(Q_OS_WIN)
    // Windows下需要延迟通知
    if (!m_resizeNotifyTimer) {
        m_resizeNotifyTimer = new QTimer(this);
        m_resizeNotifyTimer->setSingleShot(true);
        connect(m_resizeNotifyTimer, &QTimer::timeout, this, [this]() {
            if (m_playerHandle >= 0) {
                // TODO: 获取实际窗口尺寸
                PicPlayer_SetWindowSize(m_playerHandle, 800, 600);
            }
        });
    }
    m_resizeNotifyTimer->start(100);
#else
    // macOS直接通知
    PicPlayer_SetWindowSize(m_playerHandle, 800, 600);
#endif
}

void PicMatchViewModel::updateUIState()
{
    if (m_running) {
        m_statusText = "运行中";
    } else {
        m_statusText = "";
    }
    emit statusTextChanged(m_statusText);
}

// ==================== PicPlayer/PicRecognition 初始化 ====================

void PicMatchViewModel::initPicPlayer()
{
    if (m_playerHandle != -1)
        return;

    PicPlayer_Init();
    m_playerHandle = PicPlayer_CreateInstance();
    if (m_playerHandle == -1) {
        LOG_ERROR("PicPlayer_CreateInstance failed!");
        return;
    }

    PicPlayer_RegisterCallback(m_playerHandle, picCallbackBridge, this);

    // 注册窗口 - 通过查找QML窗口获取winId
    // 注意：窗口注册需要在QML加载完成后由PlayerHostItem处理
    // 这里暂时不注册窗口，由外部调用registerWindow处理

    PicPlayer_Play(m_playerHandle);
}

void PicMatchViewModel::destroyPicPlayer()
{
    if (m_playerHandle != -1) {
        PicPlayer_DestroyInstance(m_playerHandle);
        m_playerHandle = -1;
        emit handleChanged(-1);
    }
    PicPlayer_UnInit();
}

void PicMatchViewModel::initFaceRecognition(const QString& binPath)
{
    InitFaceRecognition(binPath.toLocal8Bit().constData());
}

void PicMatchViewModel::destroyFaceRecognition()
{
    DestroyFaceRecognition();
}

// ==================== 回调处理 ====================

void PicMatchViewModel::onPlayerCallback(int handle, int msg, void* data)
{
    if (handle != m_playerHandle)
        return;

    if (msg == Callback_ShowPicId) {
        if (data) {
            std::string showid(reinterpret_cast<const char*>(data));
            LOG_DEBUG("onPlayerCallback showid: {}", showid);
            const QString showIdQ = QString::fromStdString(showid);

            // 仅在此处根据「刚完全展示」的 showId 恢复人脸并刷脸区
            if (m_faceCacheByShowId.contains(showIdQ)) {
                const QList<FaceData> list = m_faceCacheByShowId.value(showIdQ);
                m_model->clearFaces();
                for (const FaceData& f : list)
                    m_model->addFace(f);
                emit faceListChanged();
            }

            // 获取下一张图片（人脸清空和列表更新在onImageUpdated中处理）
            QString nextImage = m_model->getNextImageName();
            if (!nextImage.isEmpty() && nextImage != "default_image") {
                const QString basePath = m_model->getCurrentDataPath().isEmpty() ?
                    getDefaultDataPath() : m_model->getCurrentDataPath();
                const QString imagePath = basePath + QChar('/') + nextImage;
                if (QFileInfo::exists(imagePath)) {
                    const QString newShowId = QFileInfo(nextImage).baseName();
                    onImageUpdated(newShowId, imagePath);
                }
            }
        }
    }
}

void PicMatchViewModel::onImageUpdated(const QString& showId, const QString& imagePath)
{
    if (m_playerHandle < 0)
        return;

    // 更新当前showId
    m_model->setCurrentShowId(showId);

    // 加载图片
    std::unique_ptr<PicShowInfo> picData = std::make_unique<PicShowInfo>();
    std::unique_ptr<FaceDetectionResult> faceResult = std::make_unique<FaceDetectionResult>();

    // 设置图片数据
    strncpy(picData->imageId, showId.toStdString().c_str(), IMAGE_ID_LEN - 1);
    picData->imageId[IMAGE_ID_LEN - 1] = '\0';
    picData->picReadTime = 1;

    strncpy(faceResult->imageId, showId.toStdString().c_str(), IMAGE_ID_LEN - 1);
    faceResult->imageId[IMAGE_ID_LEN - 1] = '\0';

    // 加载JPEG到RGBA
    int channels, picWidth, picHeight;
    unsigned char* imageData = stbi_load(imagePath.toStdString().c_str(), &picWidth, &picHeight, &channels, 4);
    if (!imageData) {
        LOG_ERROR("Failed to load image: {}", imagePath.toStdString());
        return;
    }

    // 旋转90度
    unsigned char* rotatedData = nullptr;
    int newWidth = picHeight;
    int newHeight = picWidth;
    rotatedData = static_cast<unsigned char*>(malloc(newWidth * newHeight * 4));
    if (!rotatedData) {
        stbi_image_free(imageData);
        LOG_ERROR("Failed to allocate memory for rotated image");
        return;
    }

    for (int y = 0; y < picHeight; ++y) {
        for (int x = 0; x < picWidth; ++x) {
            for (int c = 0; c < 4; ++c) {
                rotatedData[((newHeight - 1 - x) * newWidth + y) * 4 + c] =
                    imageData[(y * picWidth + x) * 4 + c];
            }
        }
    }
    stbi_image_free(imageData);

    // 设置数据
    picData->imageRgbaLen = newWidth * newHeight * 4;
    picData->picWidth = newWidth;
    picData->picHeight = newHeight;
    picData->imageRgbaData = static_cast<char*>(malloc(picData->imageRgbaLen));
    if (picData->imageRgbaData) {
        memcpy(picData->imageRgbaData, rotatedData, picData->imageRgbaLen);
    }
    free(rotatedData);

    // 输入图片到播放器
    if (!PicPlayer_InputPicData(m_playerHandle, 1, picData.get())) {
        LOG_ERROR("Failed to input picture data to player");
        return;
    }

    // 人脸检测
    if (DetectFacesInRgba(picData.get(), faceResult.get()) != 0) {
        LOG_ERROR("Face detection failed");
        return;
    }

    // 翻转坐标
    if (faceResult->faces && faceResult->faceCount > 0) {
        for (int i = 0; i < faceResult->faceCount; ++i) {
            faceResult->faces[i].x = 1.0f - faceResult->faces[i].x - faceResult->faces[i].width;
        }
    }

    // 输入人脸识别结果到播放器
    if (!PicPlayer_InputFaceRecogResult(m_playerHandle, faceResult.get())) {
        LOG_ERROR("Failed to input face recognition result to player");
    }

    // 更新本地人脸列表
    m_model->clearFaces();
    if (faceResult->faces && faceResult->faceCount > 0) {
        for (int i = 0; i < faceResult->faceCount; ++i) {
            FaceInfo& fi = faceResult->faces[i];
            FaceData faceData;
            faceData.id = QString("%1_face_%2").arg(showId).arg(i);
            faceData.rect = QRectF(fi.x, fi.y, fi.width, fi.height);
            faceData.attributes["confidence"] = fi.confidence;
            faceData.attributes["age"] = fi.age;

            if (fi.faceImageData && fi.faceImageLength > 0) {
                faceData.imageData = QByteArray(fi.faceImageData, fi.faceImageLength);
                faceData.width = fi.faceImageWidth;
                faceData.height = fi.faceImageHeight;
            }

            m_model->addFace(faceData);
        }
    }

    // 仅缓存人脸，不在此处 emit faceListChanged；等收到 Callback_ShowPicId 时再恢复并 emit
    m_faceCacheByShowId[showId] = m_model->faces();
    while (m_faceCacheByShowId.size() > 10)
        m_faceCacheByShowId.remove(m_faceCacheByShowId.firstKey());

    emit imageUpdated(showId, imagePath);
}

void PicMatchViewModel::setFacesFromDetectionResult(void* faceResult, const QString& showId)
{
    if (!faceResult || !m_model)
        return;
    FaceDetectionResult* result = static_cast<FaceDetectionResult*>(faceResult);
    m_model->clearFaces();
    if (result->faces && result->faceCount > 0) {
        for (int i = 0; i < result->faceCount; ++i) {
            FaceInfo& fi = result->faces[i];
            FaceData faceData;
            faceData.id = QString("%1_face_%2").arg(showId).arg(i);
            faceData.rect = QRectF(fi.x, fi.y, fi.width, fi.height);
            faceData.attributes["confidence"] = fi.confidence;
            faceData.attributes["age"] = fi.age;
            if (fi.faceImageData && fi.faceImageLength > 0) {
                faceData.imageData = QByteArray(fi.faceImageData, fi.faceImageLength);
                faceData.width = fi.faceImageWidth;
                faceData.height = fi.faceImageHeight;
            }
            m_model->addFace(faceData);
        }
    }
    emit faceListChanged();
}
