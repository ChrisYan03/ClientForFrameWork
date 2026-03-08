#ifndef PICMATCHMODEL_H
#define PICMATCHMODEL_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>
#include <QRectF>
#include <vector>

/**
 * @brief 人脸数据模型
 */
struct FaceData {
    QString id;                    // 人脸唯一标识
    QRectF rect;                  // 人脸区域
    QVariantMap attributes;        // 其他属性（年龄、性别等）
    QByteArray imageData;         // 人脸图片数据
    int width = 0;                // 图片宽度
    int height = 0;               // 图片高度

    QVariantMap toVariantMap() const {
        QVariantMap map;
        map["id"] = id;
        map["rectX"] = rect.x();
        map["rectY"] = rect.y();
        map["rectWidth"] = rect.width();
        map["rectHeight"] = rect.height();
        map["attributes"] = attributes;
        map["width"] = width;
        map["height"] = height;
        map["imageData"] = imageData;
        return map;
    }

    static FaceData fromVariantMap(const QVariantMap& map) {
        FaceData data;
        data.id = map["id"].toString();
        data.rect = QRectF(
            map["rectX"].toDouble(),
            map["rectY"].toDouble(),
            map["rectWidth"].toDouble(),
            map["rectHeight"].toDouble()
        );
        data.attributes = map["attributes"].toMap();
        data.width = map["width"].toInt();
        data.height = map["height"].toInt();
        data.imageData = map["imageData"].toByteArray();
        return data;
    }
};

/**
 * @brief 图片数据模型
 */
struct ImageData {
    QString id;                    // 图片唯一标识（showId）
    QString path;                  // 图片路径
    QSize size;                   // 图片尺寸
    QList<FaceData> faces;       // 检测到的人脸列表
    bool processed = false;       // 是否已处理

    QVariantMap toVariantMap() const {
        QVariantMap map;
        map["id"] = id;
        map["path"] = path;
        map["width"] = size.width();
        map["height"] = size.height();
        map["processed"] = processed;

        QVariantList faceList;
        for (const auto& face : faces) {
            faceList.append(face.toVariantMap());
        }
        map["faces"] = faceList;
        return map;
    }
};

/**
 * @brief PicMatch数据模型
 * 负责图片列表管理、当前索引、显示ID等数据管理
 */
class PicMatchModel : public QObject
{
    Q_OBJECT

public:
    explicit PicMatchModel(QObject* parent = nullptr);
    ~PicMatchModel();

    // 图片列表管理
    void initializeImageList(const QString& dataPath);
    void clearImageList();
    QString getNextImageName();
    QString getCurrentDataPath() const { return m_dataPath; }
    void setDataPath(const QString& path);

    // 当前状态
    QString currentShowId() const { return m_currentShowId; }
    void setCurrentShowId(const QString& id) { m_currentShowId = id; }
    size_t currentIndex() const { return m_currentIndex; }
    void resetIndex() { m_currentIndex = 0; m_imageList.clear(); m_initialized = false; }

    // 人脸数据
    void addFace(const FaceData& face);
    void clearFaces();
    QList<FaceData> faces() const { return m_faces; }

    // 序列化
    QVariantList facesToVariantList() const;

signals:
    void dataPathChanged(const QString& path);
    void imageListChanged();
    void facesChanged();

private:
    QString m_dataPath;
    QString m_currentShowId;
    QStringList m_imageList;
    size_t m_currentIndex;
    bool m_initialized;
    QList<FaceData> m_faces;

    static const char* kSettingsOrg;
    static const char* kSettingsApp;
    static const char* kSettingsDataPathKey;
};

#endif // PICMATCHMODEL_H
