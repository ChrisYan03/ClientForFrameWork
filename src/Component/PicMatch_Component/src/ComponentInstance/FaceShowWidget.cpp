#include "FaceShowWidget.h"
#include <QSizePolicy>
#include <QLabel>
#include "LogUtil.h"
#include "ResourceManager.h"
#include <memory>

FaceShowWidget::FaceShowWidget(QWidget *parent)
    : BaseWidget(parent)
{
    setObjectName("FaceShowWidget");
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    QLabel* sectionHeader = new QLabel(this);
    sectionHeader->setObjectName("FaceShowSectionHeader");
    sectionHeader->setText(QStringLiteral("识别结果"));
    m_layout->addWidget(sectionHeader);

    m_scrollArea = new QScrollArea();
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setObjectName("FaceShowScrollArea");
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    m_containerWidget = new QWidget();
    QVBoxLayout* containerLayout = new QVBoxLayout(m_containerWidget);
    containerLayout->setAlignment(Qt::AlignTop);
    containerLayout->setSpacing(8);
    containerLayout->setContentsMargins(8, 12, 8, 12);
    
    m_scrollArea->setWidget(m_containerWidget);
    m_containerWidget->setObjectName("FaceShowContainer");
    m_layout->addWidget(m_scrollArea, 1);
}

void FaceShowWidget::clearFaceImages(bool clearPending)
{
    LOG_INFO("clearFaceImages");
    if (!m_containerWidget) {
        if (clearPending) {
            for (const auto& tuple : m_pendingFaceLabels) {
                delete std::get<0>(tuple);
                // Use smart pointer to ensure proper cleanup
                ClientForFrame::ArrayPtr<char> cleanupData(std::get<1>(tuple));
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
    if (clearPending) {
        for (const auto& tuple : m_pendingFaceLabels) {
            delete std::get<0>(tuple);
            // Use smart pointer to ensure proper cleanup
            ClientForFrame::ArrayPtr<char> cleanupData(std::get<1>(tuple));
        }
        m_pendingFaceLabels.clear();
    }
    this->repaint();
}

void FaceShowWidget::addFaceImages(char* faceImageData, size_t faceImageLength, int imageWidth, int imageHeight)
{
    LOG_INFO("addFaceImages - imageData: {}, length: {}, width: {}, height: {}",
             (void*)faceImageData, faceImageLength, imageWidth, imageHeight);

    // Use smart pointer for automatic memory management
    auto copiedFaceData = ClientForFrame::make_array<char>(faceImageLength);
    memcpy(copiedFaceData.get(), faceImageData, faceImageLength);

    FaceImageLabel* faceLabel = new FaceImageLabel();
    // Store the raw pointer with ownership transferred to FaceImageLabel
    m_pendingFaceLabels.push_back(std::make_tuple(faceLabel, copiedFaceData.release(), faceImageLength, imageWidth, imageHeight));
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
    m_containerWidget->update();
    m_scrollArea->update();
    this->repaint();
}
