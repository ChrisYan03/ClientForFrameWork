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
    m_scrollArea->setStyleSheet(
        "QScrollArea {"
        "    border: 1px solid #cccccc;"
        "    border-radius: 3px;"
        "    background-color: white;"
        "}"
        "QScrollBar:vertical {"
        "    border: none;"
        "    background: #f0f0f0;"
        "    width: 16px;"
        "    margin: 0px 0px 0px 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: #c0c0c0;"
        "    min-height: 20px;"
        "    border-radius: 4px;"
        "    margin: 2px 4px 2px 4px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "    background: #a0a0a0;"
        "}"
        "QScrollBar::handle:vertical:pressed {"
        "    background: #808080;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "    height: 0px;"
        "    subcontrol-position: top;"
        "    subcontrol-origin: margin;"
        "}"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
        "    background: none;"
        "}"
    );
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
        QLayoutItem* child;
        while ((child = layout->takeAt(0)) != nullptr) {
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

    // Create a deep copy of the face image data
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