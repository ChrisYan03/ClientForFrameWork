#include "PicMatchModel.h"
#include <QBuffer>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QSettings>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QUrl>
#include <QFile>
#include <QCryptographicHash>
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
    QString tempDir = QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation)).filePath(QStringLiteral("ClientForFrame_faces"));
    QDir().mkpath(tempDir);
    static quint64 fileCounter = 0;  // 静态计数器，每次调用递增
    ++fileCounter;
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
                // 使用 RGB PNG，避免上游 alpha 全透明导致 QML 已加载但看不到图像。
                QImage exportImg = img.copy().convertToFormat(QImage::Format_RGB888);
                QByteArray pngBytes;
                QBuffer buf(&pngBytes);
                if (buf.open(QIODevice::WriteOnly) && exportImg.save(&buf, "PNG")) {
                    map["imageDataUrl"] = QStringLiteral("data:image/png;base64,") + QString::fromLatin1(pngBytes.toBase64());
                    const QString safeId = face.id.isEmpty() ? QStringLiteral("face") : face.id;
                    // 使用计数器 + idx 生成唯一文件名，确保每次都创建新文件
                    const QString fileName = QStringLiteral("%1_%2_%3.png").arg(safeId).arg(fileCounter).arg(idx);
                    QString filePath = QDir(tempDir).absoluteFilePath(fileName);
                    QFile f(filePath);
                    if (f.open(QIODevice::WriteOnly) && static_cast<qint64>(pngBytes.size()) == f.write(pngBytes)) {
                        map["imageFileUrl"] = QUrl::fromLocalFile(filePath).toString();
                        LOG_DEBUG("facesToVariantList face={} fileUrl={}", face.id.toStdString(), map["imageFileUrl"].toString().toStdString());
                    } else {
                        LOG_DEBUG("facesToVariantList write file failed face={} path={}", face.id.toStdString(), filePath.toStdString());
                    }
                }
            }
        }
        list.append(map);
        ++idx;
    }
    return list;
}
