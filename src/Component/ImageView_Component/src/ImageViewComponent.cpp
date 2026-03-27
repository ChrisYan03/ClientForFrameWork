#include "ImageViewComponent.h"
#include "ImageViewComponentGlobal.h"
#include "QmlBridge/ImageViewHostItem.h"
#include <QDir>
#include <QCoreApplication>
#include <cstring>
#include "LogUtil.h"

ImageViewComponent::ImageViewComponent(QObject* parent)
    : QObject(parent)
{
}

ImageViewComponent::~ImageViewComponent()
{
}

int ImageViewComponent::initialize(void* engine, const char* basePath)
{
    if (!engine) {
        LOG_WARN("ImageViewComponent::initialize - engine is null");
        return 0;
    }
    m_engine = static_cast<QQmlEngine*>(engine);
    m_basePath = QString::fromUtf8(basePath);
    if (!m_basePath.endsWith('/') && !m_basePath.endsWith('\\')) {
        m_basePath += '/';
    }

    if (m_manifest.id.empty()) {
        m_manifest.id = "ImageView";
        m_manifest.name = qApp->translate("ImageViewComponent", "图像预览").toStdString();
        m_manifest.version = "1.0.0";
    }

    registerQmlTypes(m_engine.data());
    m_initialized = true;
    LOG_INFO("ImageViewComponent initialized, basePath={}", m_basePath.toStdString());
    return 1;
}

void ImageViewComponent::shutdown()
{
    m_initialized = false;
}

void ImageViewComponent::registerQmlTypes(void* engine)
{
    QQmlEngine* qmlEngine = static_cast<QQmlEngine*>(engine);
    if (!qmlEngine) {
        return;
    }
    ImageViewHostItem::setComponentBasePath(m_basePath);
    qmlRegisterType<ImageViewHostItem>("ImageViewCore", 1, 0, "ImageViewHostItem");

    const QString qmlPath = m_basePath + QStringLiteral("bin/qml/");
    if (QDir(qmlPath).exists()) {
        qmlEngine->addImportPath(qmlPath);
    }
}

void* ImageViewComponent::getInterface(const char* interfaceName)
{
    (void)interfaceName;
    return nullptr;
}

void ImageViewComponent::onThemeChanged(int theme)
{
    LOG_INFO("ImageViewComponent::onThemeChanged - theme={}", theme);
}

void ImageViewComponent::onLanguageChanged(int language)
{
    LOG_INFO("ImageViewComponent::onLanguageChanged - language={}", language);
}

extern "C" IMAGEVIEWCOMPONENT_API void* IMAGEVIEWCOMPONENT_CALL createComponent()
{
    return new ImageViewComponent();
}
