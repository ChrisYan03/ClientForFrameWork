#include "FaceShowWidget.h"
#include <QSizePolicy>
#include "LogUtil.h"

FaceShowWidget::FaceShowWidget(QWidget *parent)
    : BaseWidget(parent)
{ 
    // Create main layout
    m_layout = new QVBoxLayout(this);
    
    // Create scroll area
    m_scrollArea = new QScrollArea();
    m_scrollArea->setWidgetResizable(true);
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

void FaceShowWidget::clearFaceImages() 
{
    LOG_INFO("clearFaceImages");
    QLayout* layout = m_containerWidget->layout();
    if (layout) {
        // Delete all widgets in the layout
        while (QLayoutItem* child = layout->takeAt(0)) {
            if (child->widget()) {
                delete child->widget();
            }
            delete child;
        }
        // Also clear the internal vector
        m_faceLabels.clear();

        // 刷新界面以确保清除效果立即可见
        m_containerWidget->update();
        m_scrollArea->update();
        this->repaint();
    }
}

void FaceShowWidget::addFaceImages(char* faceImageData, size_t faceImageLength, int imageWidth, int imageHeight) 
{
    LOG_INFO("addFaceImages - imageData: {}, length: {}, width: {}, height: {}", 
             (void*)faceImageData, faceImageLength, imageWidth, imageHeight);

    FaceImageLabel* faceLabel = new FaceImageLabel();
    faceLabel->setFaceInfo(faceImageData, faceImageLength, imageWidth, imageHeight);
    static_cast<QVBoxLayout*>(m_containerWidget->layout())->addWidget(faceLabel);
    m_faceLabels.push_back(faceLabel);

    // 刷新界面以确保新添加的标签立即可见
    m_containerWidget->update();
    m_scrollArea->update();
    this->repaint();
}