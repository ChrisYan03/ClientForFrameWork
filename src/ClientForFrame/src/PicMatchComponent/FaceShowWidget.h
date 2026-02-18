#ifndef FACESHOWWIDGET_H
#define FACESHOWWIDGET_H

#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QScrollArea>
#include "../Common/BaseWidget.h"  // Include BaseWidget header

class FaceImageLabel : public QLabel {
    Q_OBJECT
public:
    explicit FaceImageLabel(QWidget*parent = nullptr)
        : QLabel(parent)
        , m_faceImageData(nullptr)
        , m_faceImageLength(0)
        , m_faceImageWidth(0)
        , m_faceImageHeight(0)
 {
        setStyleSheet("margin: 5px;");
        setAlignment(Qt::AlignCenter);
    }

    //实现析构函数释放内存
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
            m_faceImageData = new char[faceImageLength];
            memcpy(m_faceImageData, faceImageData, faceImageLength);
            m_faceImageLength = faceImageLength;
            m_faceImageWidth = imageWidth;
            m_faceImageHeight = imageHeight;
            updateDisplay();
        } else {
            setText("Invalid face image data");
        }
    }

    QSize sizeHint() const override {
        return QSize(200, 250);  // Suggested size for face image widget
    }

private:
    void updateDisplay() {
        if (m_faceImageData && m_faceImageWidth > 0 && m_faceImageHeight > 0) 
        {
            int bytesPerLine = m_faceImageWidth * 4; // 假设RGBA格式
            QImage image(reinterpret_cast<const uchar*>(m_faceImageData), 
                         m_faceImageWidth, 
                         m_faceImageHeight, 
                         bytesPerLine, 
                         QImage::Format_RGBA8888);
            
            if (!image.isNull()) {
                // 验证图像格式是否符合预期
                if (image.format() != QImage::Format_RGBA8888) {
                    // 如需要则转换为预期格式
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
    char* m_faceImageData; // Cropped face image data in RGBA format
    size_t m_faceImageLength; // Length of the face image data
    int m_faceImageWidth;  // Width of the cropped face image
    int m_faceImageHeight; // Height of the cropped face image

};

class FaceShowWidget : public BaseWidget {
    Q_OBJECT

public:
    explicit FaceShowWidget(QWidget *parent = nullptr);

    // Method to clear all displayed face images
    void clearFaceImages();

    // Method to add multiple face images at once
    void addFaceImages(char* faceImageData, size_t faceImageLength, int imageWidth, int imageHeight);

    void triggerDisplay();

private:
    bool m_shouldDisplay;
    QVBoxLayout* m_layout;
    QScrollArea* m_scrollArea;
    QWidget* m_containerWidget;
    QVector<FaceImageLabel*> m_faceLabels;
    QVector<std::tuple<FaceImageLabel*, char*, size_t, int, int>> m_pendingFaceLabels;   // Store face display labels
};

#endif // FACESHOWWIDGET_H
