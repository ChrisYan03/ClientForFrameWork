/**
 * @file EmbeddedExeComponentLoader.cpp
 * @brief 嵌入式可执行文件组件加载器实现
 */
#include "EmbeddedExeComponentLoader.h"
#include <QDir>
#include <QProcess>
#include "LogUtil.h"

// ==================== EmbeddedExeWrapper 实现 ====================

EmbeddedExeWrapper::EmbeddedExeWrapper(const QString& executablePath, QObject* parent)
    : QObject(parent)
    , m_executablePath(executablePath)
{
    m_process = new QProcess(this);

    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        emit processStateChanged(false);
    });

    connect(m_process, &QProcess::started, this, [this]() {
        emit processStateChanged(true);
    });

    connect(m_process, &QProcess::readyReadStandardOutput, this, [this]() {
        QByteArray output = m_process->readAllStandardOutput();
        emit messageReceived(QString::fromUtf8(output));
    });
}

EmbeddedExeWrapper::~EmbeddedExeWrapper()
{
    stop();
}

bool EmbeddedExeWrapper::start()
{
    if (m_process->state() == QProcess::Running) {
        return true; // 已经在运行
    }

    if (!QFile::exists(m_executablePath)) {
        LOG_WARN("Executable not found: {}", m_executablePath.toStdString());
        return false;
    }

    m_process->start(m_executablePath);
    return m_process->waitForStarted(5000);
}

bool EmbeddedExeWrapper::stop()
{
    if (m_process->state() == QProcess::NotRunning) {
        return true; // 已经停止
    }

    m_process->terminate();
    return m_process->waitForFinished(5000);
}

bool EmbeddedExeWrapper::isRunning() const
{
    return m_process->state() == QProcess::Running;
}

// ==================== EmbeddedExeComponentLoader 实现 ====================

EmbeddedExeComponentLoader::EmbeddedExeComponentLoader(QObject* parent)
    : QObject(parent)
{
}

EmbeddedExeComponentLoader::~EmbeddedExeComponentLoader()
{
    // 清理所有嵌入式组件
    qDeleteAll(m_loadedEmbeddedComponents);
    m_loadedEmbeddedComponents.clear();
}

QObject* EmbeddedExeComponentLoader::load(const ComponentManifest& manifest, const QString& basePath)
{
    QString executablePath = resolveExecutablePath(manifest, basePath);
    if (executablePath.isEmpty() || !QFile::exists(executablePath)) {
        LOG_WARN("Executable not found: {}", executablePath.toStdString());
        return nullptr;
    }

    EmbeddedExeWrapper* wrapper = new EmbeddedExeWrapper(executablePath, this);

    // 启动嵌入式进程
    if (!wrapper->start()) {
        LOG_WARN("Failed to start embedded executable: {}", executablePath.toStdString());
        delete wrapper;
        return nullptr;
    }

    QString componentId = QString::fromStdString(manifest.id);
    m_loadedEmbeddedComponents[componentId] = wrapper;

    return wrapper;
}

bool EmbeddedExeComponentLoader::unload(QObject* component)
{
    EmbeddedExeWrapper* wrapper = qobject_cast<EmbeddedExeWrapper*>(component);
    if (!wrapper) {
        return false;
    }

    // 停止进程
    wrapper->stop();

    // 从管理器中移除
    for (auto it = m_loadedEmbeddedComponents.begin(); it != m_loadedEmbeddedComponents.end(); ++it) {
        if (it.value() == wrapper) {
            m_loadedEmbeddedComponents.erase(it);
            break;
        }
    }

    delete wrapper;
    return true;
}

bool EmbeddedExeComponentLoader::canLoad(const ComponentManifest& manifest, const QString& basePath) const
{
    QString executablePath = resolveExecutablePath(manifest, basePath);
    return QFile::exists(executablePath);
}

QString EmbeddedExeComponentLoader::resolveExecutablePath(const ComponentManifest& manifest, const QString& basePath) const
{
    QString executablePath = basePath + QStringLiteral("/bin/");

    // 从manifest中获取可执行文件名，如果没有则使用组件ID
    QString executableName;
    if (!manifest.executable.empty()) {
        executableName = QString::fromStdString(manifest.executable);
    } else {
        executableName = QString::fromStdString(manifest.id) + QStringLiteral(".exe");
    }

    executablePath += executableName;
    return executablePath;
}

QString EmbeddedExeComponentLoader::extractUrl(const ComponentManifest& manifest) const
{
    // TODO: 从manifest中提取URL
    return QString();
}
