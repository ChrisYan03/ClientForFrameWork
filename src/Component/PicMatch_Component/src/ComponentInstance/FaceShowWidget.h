#ifndef FACESHOWWIDGET_H
#define FACESHOWWIDGET_H

#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QScrollArea>
#include "BaseWidget.h"

class FaceImageLabel : public QLabel {
    Q_OBJECT
public:
    explicit FaceImageLabel(QWidget* parent = nullptr)
        : QLabel(parent)
        , m_faceImageData(nullptr)
        , m_faceImageLength(0)
        , m_faceImageWidth(0)
        , m_faceImageHeight(0)
    {
        setObjectName("FaceImageLabel");
        setAlignment(Qt::AlignCenter);
    }

    ~FaceImageLabel() {
        if (m_faceImageData) {
            delete[] m_faceImageData;
            m_faceImageData = nullptr;
        }
    }

    void setFaceInfo(char* faceImageData, size_t faceImageLength, int imageWidth, int imageHeight) {
        if (faceImageData && faceImageLength > 0 && imageWidth > 0 && imageHeight > 0) {
            if (m_faceImageData) {
                delete[] m_faceImageData;
                m_faceImageData = nullptr;
            }
            m_faceImageData = faceImageData;
            m_faceImageLength = faceImageLength;
            m_faceImageWidth = imageWidth;
            m_faceImageHeight = imageHeight;
            updateDisplay();
        } else {
            setText("Invalid face image data");
            if (faceImageData) {
                delete[] faceImageData;
            }
        }
    }

    QSize sizeHint() const override {
        return QSize(200, 250);
    }

private:
    void updateDisplay() {
        if (m_faceImageData && m_faceImageWidth > 0 && m_faceImageHeight > 0) 
        {
            int bytesPerLine = m_faceImageWidth * 4;
            QImage image(reinterpret_cast<const uchar*>(m_faceImageData), 
                         m_faceImageWidth, 
                         m_faceImageHeight, 
                         bytesPerLine, 
                         QImage::Format_RGBA8888);
            
            if (!image.isNull()) {
                if (image.format() != QImage::Format_RGBA8888) {
                    QImage convertedImage = image.convertToFormat(QImage::Format_RGBA8888);
                    QPixmap pixmap = QPixmap::fromImage(convertedImage.mirrored(true, false));
                    setPixmap(pixmap.scaled(180, 180, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                } else {
                    QPixmap pixmap = QPixmap::fromImage(image.mirrored(true, false));
                    setPixmap(pixmap.scaled(180, 180, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                }
            } 
        } else {
            setText("无面部图像");
        }
    }

private:
    char* m_faceImageData;
    size_t m_faceImageLength;
    int m_faceImageWidth;
    int m_faceImageHeight;
};

class FaceShowWidget : public BaseWidget {
    Q_OBJECT

public:
    explicit FaceShowWidget(QWidget *parent = nullptr);

    void clearFaceImages(bool clearPending = false);
    void addFaceImages(char* faceImageData, size_t faceImageLength, int imageWidth, int imageHeight);
    void triggerDisplay();

private:
    bool m_shouldDisplay;
    QVBoxLayout* m_layout;
    QScrollArea* m_scrollArea;
    QWidget* m_containerWidget;
    QVector<FaceImageLabel*> m_faceLabels;
    QVector<std::tuple<FaceImageLabel*, char*, size_t, int, int>> m_pendingFaceLabels;
};

#endif // FACESHOWWIDGET_H
