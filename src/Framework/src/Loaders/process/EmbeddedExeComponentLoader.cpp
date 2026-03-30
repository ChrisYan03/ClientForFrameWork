/**
 * @file EmbeddedExeComponentLoader.cpp
 * @brief 嵌入式可执行文件组件加载器实现
 */
#include "EmbeddedExeComponentLoader.h"
#include <QDir>
#include <QProcess>
#include "LogUtil.h"

EmbeddedExeWrapper::EmbeddedExeWrapper(const QString& executablePath, QObject* parent)
    : QObject(parent)
    , m_executablePath(executablePath)
{
    m_process = new QProcess(this);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int, QProcess::ExitStatus) { emit processStateChanged(false); });
    connect(m_process, &QProcess::started, this, [this]() { emit processStateChanged(true); });
    connect(m_process, &QProcess::readyReadStandardOutput, this, [this]() {
        emit messageReceived(QString::fromUtf8(m_process->readAllStandardOutput()));
    });
}

EmbeddedExeWrapper::~EmbeddedExeWrapper()
{
    stop();
}

bool EmbeddedExeWrapper::start()
{
    if (m_process->state() == QProcess::Running)
        return true;
    if (!QFile::exists(m_executablePath)) {
        LOG_WARN("Executable not found: {}", m_executablePath.toStdString());
        return false;
    }
    m_process->start(m_executablePath);
    return m_process->waitForStarted(5000);
}

bool EmbeddedExeWrapper::stop()
{
    if (m_process->state() == QProcess::NotRunning)
        return true;
    m_process->terminate();
    return m_process->waitForFinished(5000);
}

bool EmbeddedExeWrapper::isRunning() const
{
    return m_process->state() == QProcess::Running;
}

EmbeddedExeComponentLoader::EmbeddedExeComponentLoader(QObject* parent)
    : QObject(parent)
{
}

EmbeddedExeComponentLoader::~EmbeddedExeComponentLoader()
{
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
    if (!wrapper->start()) {
        LOG_WARN("Failed to start embedded executable: {}", executablePath.toStdString());
        delete wrapper;
        return nullptr;
    }

    m_loadedEmbeddedComponents[QString::fromStdString(manifest.id)] = wrapper;
    return wrapper;
}

bool EmbeddedExeComponentLoader::unload(QObject* component)
{
    EmbeddedExeWrapper* wrapper = qobject_cast<EmbeddedExeWrapper*>(component);
    if (!wrapper)
        return false;
    wrapper->stop();
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
    return QFile::exists(resolveExecutablePath(manifest, basePath));
}

QString EmbeddedExeComponentLoader::resolveExecutablePath(const ComponentManifest& manifest, const QString& basePath) const
{
    QString executablePath = basePath + QStringLiteral("/bin/");
    QString executableName = !manifest.executable.empty()
        ? QString::fromStdString(manifest.executable)
        : (QString::fromStdString(manifest.id) + QStringLiteral(".exe"));
    executablePath += executableName;
    return executablePath;
}

QString EmbeddedExeComponentLoader::extractUrl(const ComponentManifest&) const
{
    return QString();
}
