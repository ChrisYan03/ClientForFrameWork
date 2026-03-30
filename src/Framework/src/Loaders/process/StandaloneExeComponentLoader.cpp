/**
 * @file StandaloneExeComponentLoader.cpp
 * @brief 独立可执行文件组件加载器实现
 */
#include "StandaloneExeComponentLoader.h"
#include <QDir>
#include <QProcess>
#include "LogUtil.h"

StandaloneExeWrapper::StandaloneExeWrapper(const QString& executablePath, QObject* parent)
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

StandaloneExeWrapper::~StandaloneExeWrapper()
{
    stop();
}

bool StandaloneExeWrapper::start()
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

bool StandaloneExeWrapper::stop()
{
    if (m_process->state() == QProcess::NotRunning)
        return true;
    m_process->terminate();
    return m_process->waitForFinished(5000);
}

bool StandaloneExeWrapper::isRunning() const
{
    return m_process->state() == QProcess::Running;
}

void StandaloneExeWrapper::sendMessage(const QString& message)
{
    if (isRunning())
        m_process->write(message.toUtf8() + "\n");
}

StandaloneExeComponentLoader::StandaloneExeComponentLoader(QObject* parent)
    : QObject(parent)
{
}

StandaloneExeComponentLoader::~StandaloneExeComponentLoader()
{
    qDeleteAll(m_runningProcesses);
    m_runningProcesses.clear();
}

QObject* StandaloneExeComponentLoader::load(const ComponentManifest& manifest, const QString& basePath)
{
    QString executablePath = resolveExecutablePath(manifest, basePath);
    if (executablePath.isEmpty() || !QFile::exists(executablePath)) {
        LOG_WARN("Executable not found: {}", executablePath.toStdString());
        return nullptr;
    }

    StandaloneExeWrapper* wrapper = new StandaloneExeWrapper(executablePath, this);
    if (!wrapper->start()) {
        LOG_WARN("Failed to start executable: {}", executablePath.toStdString());
        delete wrapper;
        return nullptr;
    }

    m_runningProcesses[QString::fromStdString(manifest.id)] = wrapper;
    return wrapper;
}

bool StandaloneExeComponentLoader::unload(QObject* component)
{
    StandaloneExeWrapper* wrapper = qobject_cast<StandaloneExeWrapper*>(component);
    if (!wrapper)
        return false;
    wrapper->stop();
    for (auto it = m_runningProcesses.begin(); it != m_runningProcesses.end(); ++it) {
        if (it.value() == wrapper) {
            m_runningProcesses.erase(it);
            break;
        }
    }
    delete wrapper;
    return true;
}

bool StandaloneExeComponentLoader::canLoad(const ComponentManifest& manifest, const QString& basePath) const
{
    return QFile::exists(resolveExecutablePath(manifest, basePath));
}

QString StandaloneExeComponentLoader::resolveExecutablePath(const ComponentManifest& manifest, const QString& basePath) const
{
    QString executablePath = basePath + QStringLiteral("/bin/");
    QString executableName = !manifest.executable.empty()
        ? QString::fromStdString(manifest.executable)
        : (QString::fromStdString(manifest.id) + QStringLiteral(".exe"));
    executablePath += executableName;
    return executablePath;
}
