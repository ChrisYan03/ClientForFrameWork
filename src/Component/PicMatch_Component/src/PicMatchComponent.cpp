#include "PicMatchComponent.h"
#include "PicMatchComponentGlobal.h"
#include "QmlBridge/PlayerHostItem.h"
#include "QmlBridge/PicMatchViewModel.h"
#include "QmlBridge/FaceImageProvider.h"
#include <QDir>
#include <cstring>
#include "LogUtil.h"

// ==================== 构造函数和析构函数 ====================

PicMatchComponent::PicMatchComponent(QObject *parent)
    : QObject(parent)
{
}

PicMatchComponent::~PicMatchComponent()
{
    if (m_viewModel) {
        delete m_viewModel;
        m_viewModel = nullptr;
    }
}

// ==================== IComponent 接口实现 ====================

int PicMatchComponent::initialize(void* engine, const char* basePath)
{
    LOG_INFO("PicMatchComponent::initialize - engine: {} basePath: {}", engine, basePath);
    if (!engine) {
        LOG_WARN("PicMatchComponent::initialize - engine is null!");
        return 0;
    }

    m_engine = static_cast<QQmlEngine*>(engine);
    m_basePath = QString::fromUtf8(basePath);
    LOG_INFO("PicMatchComponent::initialize - m_basePath: {}", m_basePath.toStdString());

    // 确保路径以 / 结尾
    if (!m_basePath.endsWith('/') && !m_basePath.endsWith('\\')) {
        m_basePath += '/';
    }

    // 解析 manifest.json 获取组件信息
    // 注意：Framework 会自动填充 m_manifest，这里可以直接使用
    // 如果 m_manifest 为空，使用默认值
    LOG_INFO("PicMatchComponent::initialize - m_manifest.id: {}", m_manifest.id);
    if (m_manifest.id.empty()) {
        m_manifest.id = "picmatch";
        m_manifest.name = "图像人脸识别";
        m_manifest.version = "1.0.0";
        LOG_INFO("PicMatchComponent::initialize - using default manifest");
    }

    // 注册 QML 类型
    LOG_INFO("PicMatchComponent::initialize - calling registerQmlTypes");
    registerQmlTypes(m_engine);

    m_initialized = true;
    LOG_INFO("PicMatchComponent::initialize - completed successfully");
    return 1;
}

void PicMatchComponent::shutdown()
{
    if (!m_initialized || !m_viewModel) {
        return;
    }

    // 通知 ViewModel 关闭
    QMetaObject::invokeMethod(m_viewModel, "shutdown", Qt::DirectConnection);
    m_viewModel = nullptr;

    m_initialized = false;
}

void PicMatchComponent::registerQmlTypes(void* engine)
{
    QQmlEngine* qmlEngine = static_cast<QQmlEngine*>(engine);
    LOG_INFO("PicMatchComponent::registerQmlTypes - manifest.id: {}", m_manifest.id);
    if (!engine || m_manifest.id.empty()) {
        LOG_WARN("PicMatchComponent::registerQmlTypes - skipping registration (engine or manifest empty)");
        return;
    }

    // 注册 QML 类型
    LOG_INFO("PicMatchComponent: Registering QML types for PicMatchCore");
    qmlRegisterType<PlayerHostItem>("PicMatchCore", 1, 0, "PlayerHostItem");
    qmlRegisterType<PicMatchViewModel>("PicMatchCore", 1, 0, "PicMatchViewModel");

    // 注册图片提供者（仅注册一次）
    static bool imageProviderRegistered = false;
    if (!imageProviderRegistered) {
        qmlEngine->addImageProvider(QStringLiteral("picmatchfaces"), new FaceImageProvider());
        imageProviderRegistered = true;
        LOG_INFO("PicMatchComponent: Registered image provider 'picmatchfaces'");
    }

    // 添加组件 QML 导入路径
    // basePath指向组件根目录，DLL和QML在bin/子目录下
    QString qmlPath = m_basePath + QStringLiteral("bin/qml/");
    LOG_INFO("PicMatchComponent: Checking QML path: {} exists: {}",
            qmlPath.toStdString(), QDir(qmlPath).exists());
    if (QDir(qmlPath).exists()) {
        qmlEngine->addImportPath(qmlPath);
        LOG_INFO("PicMatchComponent: Added QML import path: {}", qmlPath.toStdString());
    } else {
        LOG_WARN("PicMatchComponent: QML path does not exist: {}", qmlPath.toStdString());
        // 尝试列出basePath下的目录
        QDir baseDir(m_basePath);
        auto entries = baseDir.entryList();
        QString entriesStr;
        for (const auto& entry : entries) {
            entriesStr += entry + " ";
        }
        LOG_INFO("PicMatchComponent: Contents of basePath: {}", entriesStr.toStdString());
    }
}

void* PicMatchComponent::getInterface(const char* interfaceName)
{
    if (strcmp(interfaceName, "ViewModel") == 0) {
        return m_viewModel;
    }
    return nullptr;
}

// ==================== 工厂函数 ====================

// 导出 createComponent 符号，供 Framework 加载
extern "C" PICMATCHCOMPONENT_API void* PICMATCHCOMPONENT_CALL createComponent()
{
    return new PicMatchComponent();
}
