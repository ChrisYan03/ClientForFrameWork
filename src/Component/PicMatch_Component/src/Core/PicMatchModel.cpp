#include "PicMatchModel.h"
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QSettings>
#include <QCoreApplication>
#include <QUrl>
#include "QmlBridge/FaceImageProvider.h"
#include "LogUtil.h"

const char* PicMatchModel::kSettingsOrg = "ClientForFrame";
const char* PicMatchModel::kSettingsApp = "PicMatchComponent";
const char* PicMatchModel::kSettingsDataPathKey = "DataPath";

PicMatchModel::PicMatchModel(QObject* parent)
    : QObject(parent)
    , m_currentShowId("")
    , m_currentIndex(0)
    , m_initialized(false)
{
    // 加载保存的数据路径
    QSettings s(kSettingsOrg, kSettingsApp);
    m_dataPath = s.value(kSettingsDataPathKey).toString().trimmed();
}

PicMatchModel::~PicMatchModel()
{
}

void PicMatchModel::setDataPath(const QString& path)
{
    if (m_dataPath != path) {
        m_dataPath = path;
        // 保存到设置
        QSettings s(kSettingsOrg, kSettingsApp);
        s.setValue(kSettingsDataPathKey, path);
        // 重置图片列表
        m_initialized = false;
        m_imageList.clear();
        m_currentIndex = 0;
        emit dataPathChanged(path);
    }
}

void PicMatchModel::initializeImageList(const QString& dataPath)
{
    if (!m_initialized) {
        m_imageList.clear();

        QString path = dataPath;
        if (path.isEmpty()) {
            path = m_dataPath;
        }

        // 如果路径为空，使用默认路径
        if (path.isEmpty()) {
#if defined(Q_OS_MAC)
            path = "/Users/chrisyan/ClientForFrameWork/picdata";
#else
            // Windows默认路径由外部指定
            LOG_WARN("Data path is empty, using default");
#endif
        }

        QDir directory(path);
        if (directory.exists()) {
            directory.setFilter(QDir::Files);
            directory.setNameFilters(QStringList() << "*.jpg" << "*.jpeg" << "*.png" << "*.bmp");
            directory.setSorting(QDir::Name);
            const QFileInfoList fileList = directory.entryInfoList();
            for (const QFileInfo& fileInfo : fileList) {
                m_imageList.push_back(fileInfo.fileName());
            }
            LOG_INFO("Initialized image list from \"{}\" with {} images", path.toStdString(), m_imageList.size());
        } else {
            LOG_WARN("Directory does not exist: {}", path.toStdString());
        }

        m_initialized = true;
        emit imageListChanged();
    }
}

void PicMatchModel::clearImageList()
{
    m_imageList.clear();
    m_currentIndex = 0;
    m_initialized = false;
    emit imageListChanged();
}

QString PicMatchModel::getNextImageName()
{
    // 确保图片列表已初始化
    if (!m_initialized) {
        initializeImageList(m_dataPath);
    }

    if (!m_imageList.empty() && m_currentIndex < m_imageList.size()) {
        QString fileName = m_imageList[m_currentIndex];
        m_currentIndex++;
        return fileName;
    }
    return QString();
}

void PicMatchModel::addFace(const FaceData& face)
{
    m_faces.append(face);
    emit facesChanged();
}

void PicMatchModel::clearFaces()
{
    m_faces.clear();
    emit facesChanged();
}

QVariantList PicMatchModel::facesToVariantList() const
{
    QVariantList list;
    static quint64 imageVersion = 0;
    ++imageVersion;
    int idx = 0;
    for (const auto& face : m_faces) {
        QVariantMap map = face.toVariantMap();
        size_t expectedSize = static_cast<size_t>(face.width) * static_cast<size_t>(face.height) * 4u;
        bool sizeOk = (!face.imageData.isEmpty() && face.width > 0 && face.height > 0
                      && expectedSize <= static_cast<size_t>(face.imageData.size()));
        if (sizeOk) {
            QImage img(reinterpret_cast<const uchar*>(face.imageData.constData()),
                      face.width, face.height, face.width * 4, QImage::Format_RGBA8888);
            if (!img.isNull()) {
                const QString safeId = face.id.isEmpty() ? QStringLiteral("face_%1").arg(idx) : face.id;
                // 统一用 RGB888，避免 alpha 数据异常导致“加载成功但不可见”。
                const QImage exportImg = img.copy().convertToFormat(QImage::Format_RGB888);
                if (FaceImageProvider::instance()) {
                    FaceImageProvider::instance()->setImage(safeId, exportImg);
                }
                map["imageProviderUrl"] = QStringLiteral("image://picmatchfaces/%1?v=%2")
                                              .arg(QString::fromLatin1(QUrl::toPercentEncoding(safeId)))
                                              .arg(imageVersion);
                LOG_DEBUG("facesToVariantList face={} providerUrl={}",
                          safeId.toStdString(),
                          map["imageProviderUrl"].toString().toStdString());
            }
        }
        list.append(map);
        ++idx;
    }
    return list;
}
