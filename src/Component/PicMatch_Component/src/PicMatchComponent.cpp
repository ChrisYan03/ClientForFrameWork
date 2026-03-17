#include "PicMatchComponent.h"
#include "PicMatchComponentGlobal.h"
#include "QmlBridge/PlayerHostItem.h"
#include "QmlBridge/PicMatchViewModel.h"
#include "QmlBridge/FaceImageProvider.h"
#include <QDir>

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

bool PicMatchComponent::initialize(QQmlEngine *engine, const QString &basePath)
{
    if (!engine) {
        return false;
    }

    m_engine = engine;
    m_basePath = basePath;

    // 确保路径以 / 结尾
    if (!m_basePath.endsWith('/') && !m_basePath.endsWith('\\')) {
        m_basePath += '/';
    }

    // 解析 manifest.json 获取组件信息
    // 注意：Framework 会自动填充 m_manifest，这里可以直接使用
    // 如果 m_manifest 为空，使用默认值
    if (m_manifest.id.empty()) {
        m_manifest.id = "picmatch";
        m_manifest.name = "图像人脸识别";
        m_manifest.version = "1.0.0";
    }

    // 注册 QML 类型
    registerQmlTypes(engine);

    m_initialized = true;
    return true;
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

void PicMatchComponent::registerQmlTypes(QQmlEngine *engine)
{
    if (!engine || m_manifest.id.empty()) {
        return;
    }

    // 注册 QML 类型
    qmlRegisterType<PlayerHostItem>("PicMatchCore", 1, 0, "PlayerHostItem");
    qmlRegisterType<PicMatchViewModel>("PicMatchCore", 1, 0, "PicMatchViewModel");

    // 注册图片提供者（仅注册一次）
    static bool imageProviderRegistered = false;
    if (!imageProviderRegistered) {
        engine->addImageProvider(QStringLiteral("picmatchfaces"), new FaceImageProvider());
        imageProviderRegistered = true;
    }

    // 添加组件 QML 导入路径
    QString qmlPath = m_basePath + QStringLiteral("qml/");
    if (QDir(qmlPath).exists()) {
        engine->addImportPath(qmlPath);
    }
}

QObject* PicMatchComponent::getInterface(const QString &interfaceName)
{
    if (interfaceName == QStringLiteral("ViewModel")) {
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
