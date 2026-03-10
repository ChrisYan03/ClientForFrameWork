#include "FaceImageProvider.h"

#include <QMutexLocker>
#include <QUrl>
#include "LogUtil.h"

FaceImageProvider* FaceImageProvider::s_instance = nullptr;

FaceImageProvider::FaceImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
{
    s_instance = this;
}

FaceImageProvider::~FaceImageProvider()
{
    if (s_instance == this) {
        s_instance = nullptr;
    }
}

FaceImageProvider* FaceImageProvider::instance()
{
    return s_instance;
}

void FaceImageProvider::setImage(const QString& faceId, const QImage& image)
{
    QMutexLocker locker(&m_mutex);
    m_images.insert(faceId, image);
    LOG_DEBUG("FaceImageProvider::setImage id={} size={}x{} cacheCount={}",
              faceId.toStdString(),
              image.width(),
              image.height(),
              m_images.size());
}

void FaceImageProvider::clear()
{
    QMutexLocker locker(&m_mutex);
    const int before = m_images.size();
    m_images.clear();
    LOG_DEBUG("FaceImageProvider::clear removed={} cacheCount={}", before, m_images.size());
}

QImage FaceImageProvider::requestImage(const QString& id, QSize* size, const QSize& requestedSize)
{
    const QString rawId = id;
    QString faceId = id.section('?', 0, 0);
    if (faceId.startsWith('/')) {
        faceId.remove(0, 1);
    }
    faceId = QUrl::fromPercentEncoding(faceId.toLatin1());

    QImage image;
    {
        QMutexLocker locker(&m_mutex);
        image = m_images.value(faceId);
        LOG_DEBUG("FaceImageProvider::requestImage rawId={} parsedId={} hit={} req={}x{} cached={}x{} cacheCount={}",
                  rawId.toStdString(),
                  faceId.toStdString(),
                  !image.isNull(),
                  requestedSize.width(),
                  requestedSize.height(),
                  image.width(),
                  image.height(),
                  m_images.size());
    }

    if (size)
        *size = image.size();

    if (image.isNull() || !requestedSize.isValid())
        return image;

    return image.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}
