#ifndef FACEIMAGEPROVIDER_H
#define FACEIMAGEPROVIDER_H

#include <QHash>
#include <QImage>
#include <QMutex>
#include <QQuickImageProvider>

class FaceImageProvider : public QQuickImageProvider
{
public:
    FaceImageProvider();
    ~FaceImageProvider() override;

    static FaceImageProvider* instance();

    void setImage(const QString& faceId, const QImage& image);
    void clear();

    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override;

private:
    static FaceImageProvider* s_instance;
    QHash<QString, QImage> m_images;
    QMutex m_mutex;
};

#endif // FACEIMAGEPROVIDER_H
