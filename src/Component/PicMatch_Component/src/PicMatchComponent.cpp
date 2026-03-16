#include "PicMatchComponent.h"
#include "PicMatchComponentApi.h"
#include "PicMatchComponentGlobal.h"
#include "QmlBridge/PlayerHostItem.h"
#include "QmlBridge/PicMatchViewModel.h"
#include "QmlBridge/FaceImageProvider.h"
#include <QQmlEngine>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <IComponentApi.h>
#include <IComponentData.h>

// ==================== 构造函数和析构函数 ====================

PicMatchComponent::PicMatchComponent(QObject *parent)
    : QObject(parent)
{
    m_state = ComponentState_Loading;
}

PicMatchComponent::~PicMatchComponent()
{
    // Simple cleanup without calling virtual methods
    if (m_viewModel) {
        delete m_viewModel;
        m_viewModel = nullptr;
    }
}

// ==================== 组件接口实现 ====================

QString PicMatchComponent::id() const
{
    return m_id;
}

QString PicMatchComponent::name() const
{
    return m_name;
}

QString PicMatchComponent::version() const
{
    return m_version;
}

QString PicMatchComponent::description() const
{
    return m_description;
}

QString PicMatchComponent::author() const
{
    return m_author;
}

QString PicMatchComponent::iconPath() const
{
    if (!m_iconPath.isEmpty()) {
        return m_iconPath;
    }
    // 默认值：从 meta_info 目录查找
    return m_basePath + QStringLiteral("meta_info/face_recognition.svg");
}

QString PicMatchComponent::qmlPage() const
{
    return m_qmlPage;
}

ComponentType PicMatchComponent::type() const
{
    return m_type;
}

bool PicMatchComponent::isInitialized() const
{
    return m_qmlTypesRegistered && m_viewModel != nullptr;
}

ComponentState PicMatchComponent::state() const
{
    return m_state;
}

void PicMatchComponent::setState(ComponentState state)
{
    m_state = state;
}

bool PicMatchComponent::initialize(QQmlEngine *engine, const QString &basePath)
{
    if (!engine) {
        m_state = ComponentState_Error;
        return false;
    }

    m_engine = engine;
    m_basePath = basePath;

    // 确保路径以 / 结尾
    if (!m_basePath.endsWith('/') && !m_basePath.endsWith('\\')) {
        m_basePath += '/';
    }

    // 从 manifest.json 加载元信息（使用 Framework 提供的工具）
    if (!loadMetaInfo(basePath)) {
        // 加载失败但继续初始化，使用默认值
    }

    // 注册 QML 类型
    registerQmlTypesInternal(engine);

    m_state = ComponentState_Loaded;

    return true;
}

void PicMatchComponent::shutdown()
{
    // Use isInitialized() to avoid virtual call issues
    if (!isInitialized() || !m_viewModel) {
        return;
    }

    m_state = ComponentState_Shutdown;

    // 清理 ViewModel
    if (m_viewModel) {
        // 通知 ViewModel 关闭
        QMetaObject::invokeMethod(m_viewModel, "shutdown", Qt::DirectConnection);
        m_viewModel = nullptr;
    }

    m_playerHostItem = nullptr;
    m_qmlTypesRegistered = false;
}

void PicMatchComponent::registerQmlTypes(QQmlEngine *engine)
{
    registerQmlTypesInternal(engine);
}

QObject* PicMatchComponent::getInterface(const QString &interfaceName)
{
    if (interfaceName == QStringLiteral("ViewModel")) {
        return m_viewModel;
    }
    return nullptr;
}

// ==================== 私有方法 ====================

bool PicMatchComponent::loadMetaInfo(const QString &basePath)
{
    QString manifestPath = basePath;
    if (!manifestPath.endsWith('/') && !manifestPath.endsWith('\\')) {
        manifestPath += '/';
    }
    manifestPath += QStringLiteral("meta_info/manifest.json");

    // 使用 Framework 提供的 loadComponentManifest 工具函数
    ComponentManifest manifest;
    if (!loadComponentManifest(manifestPath, manifest)) {
        return false;
    }

    // 直接使用解析好的结构体
    m_id = QString::fromStdString(manifest.id);
    m_name = QString::fromStdString(manifest.name);
    m_version = QString::fromStdString(manifest.version);
    m_description = QString::fromStdString(manifest.description);
    m_author = QString::fromStdString(manifest.author);

    // 解析图标路径
    QString iconRelPath = QString::fromStdString(manifest.icon);
    if (!iconRelPath.isEmpty() && !iconRelPath.startsWith('/') && !iconRelPath.startsWith("qrc:")) {
        m_iconPath = basePath + QStringLiteral("meta_info/") + iconRelPath;
    } else {
        m_iconPath = iconRelPath;
    }

    m_qmlPage = QString::fromStdString(manifest.qmlPage);

    // 解析组件类型
    QString typeStr = QString::fromStdString(manifest.type).toLower();
    if (typeStr == QStringLiteral("nativedll")) {
        m_type = ComponentType_NativeDll;
    } else if (typeStr == QStringLiteral("standaloneexe")) {
        m_type = ComponentType_StandaloneExe;
    } else if (typeStr == QStringLiteral("weburl")) {
        m_type = ComponentType_WebUrl;
    } else if (typeStr == QStringLiteral("embeddedexe")) {
        m_type = ComponentType_EmbeddedExe;
    } else {
        // 默认为原生DLL
        m_type = ComponentType_NativeDll;
    }

    return true;
}

void PicMatchComponent::registerQmlTypesInternal(QQmlEngine *engine)
{
    if (!engine || m_qmlTypesRegistered) {
        return;
    }

    // 注册 QML 类型
    // PlayerHostItem - 在 QML 中可实例化
    qmlRegisterType<PlayerHostItem>("PicMatchCore", 1, 0, "PlayerHostItem");

    // PicMatchViewModel - 可实例化类型
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

    m_qmlTypesRegistered = true;
}

// ==================== 额外接口实现 ====================

QObject* PicMatchComponent::getViewModel() const
{
    return m_viewModel;
}

QString PicMatchComponent::playerHostItemClassName()
{
    return QStringLiteral("PlayerHostItem");
}

QString PicMatchComponent::viewModelClassName()
{
    return QStringLiteral("PicMatchViewModel");
}

// ==================== 工厂函数 ====================

// 导出 createComponent 符号，供 FrameworkComponentLoader 通过 Framework API 加载
// 返回 void* 以避免编译时依赖
PICMATCHCOMPONENT_API void* PICMATCHCOMPONENT_CALL createComponent()
{
    return new PicMatchComponent();
}
