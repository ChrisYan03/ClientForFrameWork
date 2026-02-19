#include "FaceShowWidget.h"
#include <QSizePolicy>
#include "LogUtil.h"

FaceShowWidget::FaceShowWidget(BaseWidget *parent)
    : BaseWidget(parent)
{
    setObjectName("FaceShowWidget");
    // Create main layout
    m_layout = new QVBoxLayout(this);
    
    // Create scroll area
    m_scrollArea = new QScrollArea();
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setObjectName("FaceShowScrollArea");
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    // Create container widget for face images
    m_containerWidget = new QWidget();
    QVBoxLayout* containerLayout = new QVBoxLayout(m_containerWidget);
    containerLayout->setAlignment(Qt::AlignTop);  // Align to top so images stack vertically
    containerLayout->setSpacing(10);
    containerLayout->setContentsMargins(10, 10, 10, 10);
    
    m_scrollArea->setWidget(m_containerWidget);
    
    // Add scroll area to main layout
    m_layout->addWidget(m_scrollArea);
    m_layout->setContentsMargins(0, 0, 0, 0);
}

void FaceShowWidget::clearFaceImages(bool clearPending) 
{
    LOG_INFO("clearFaceImages");
    if (!m_containerWidget) {
        if (clearPending) {
            for (const auto& tuple : m_pendingFaceLabels) {
                delete std::get<0>(tuple);
                delete[] std::get<1>(tuple);
            }
            m_pendingFaceLabels.clear();
        }
        return;
    }
    QLayout* layout = m_containerWidget->layout();
    if (layout) {
        QLayoutItem* child;
        while ((child = layout->takeAt(0)) != nullptr) {
            if (child->widget()) {
                delete child->widget();
            }
            delete child;
        }
        m_faceLabels.clear();
        m_containerWidget->update();
        m_scrollArea->update();
    }
    
    this->repaint();
}

void FaceShowWidget::addFaceImages(char* faceImageData, size_t faceImageLength, int imageWidth, int imageHeight) 
{
    LOG_INFO("addFaceImages - imageData: {}, length: {}, width: {}, height: {}", 
             (void*)faceImageData, faceImageLength, imageWidth, imageHeight);

    // 深拷贝一份，所有权将在 triggerDisplay 中转移给 FaceImageLabel
    char* copiedFaceData = new char[faceImageLength];
    memcpy(copiedFaceData, faceImageData, faceImageLength);

    FaceImageLabel* faceLabel = new FaceImageLabel();
    m_pendingFaceLabels.push_back(std::make_tuple(faceLabel, copiedFaceData, faceImageLength, imageWidth, imageHeight));
}

void FaceShowWidget::triggerDisplay()
{
    LOG_INFO("triggerDisplay");
    for (const auto& tuple : m_pendingFaceLabels) {
        FaceImageLabel* faceLabel = std::get<0>(tuple);
        char* faceImageData = std::get<1>(tuple);
        size_t faceImageLength = std::get<2>(tuple);
        int imageWidth = std::get<3>(tuple);
        int imageHeight = std::get<4>(tuple);

        faceLabel->setFaceInfo(faceImageData, faceImageLength, imageWidth, imageHeight);
        static_cast<QVBoxLayout*>(m_containerWidget->layout())->addWidget(faceLabel);
        m_faceLabels.append(faceLabel);  
    }

    m_pendingFaceLabels.clear();
    // 刷新界面以确保新添加的标签立即可见
    m_containerWidget->update();
    m_scrollArea->update();
    this->repaint();
}