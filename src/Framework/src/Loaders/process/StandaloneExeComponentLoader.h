/**
 * @file StandaloneExeComponentLoader.h
 * @brief 独立可执行文件组件加载器
 */
#ifndef STANDALONE_EXE_COMPONENT_LOADER_H
#define STANDALONE_EXE_COMPONENT_LOADER_H

#include "../../IComponentLoader.h"
#include <QProcess>
#include <QMap>

class StandaloneExeWrapper : public QObject
{
    Q_OBJECT
public:
    explicit StandaloneExeWrapper(const QString& executablePath, QObject* parent = nullptr);
    ~StandaloneExeWrapper() override;

    bool start();
    bool stop();
    bool isRunning() const;
    void sendMessage(const QString& message);

signals:
    void processStateChanged(bool running);
    void messageReceived(const QString& message);

private:
    QProcess* m_process;
    QString m_executablePath;
};

class StandaloneExeComponentLoader : public QObject, public IComponentLoader
{
    Q_OBJECT
public:
    explicit StandaloneExeComponentLoader(QObject* parent = nullptr);
    ~StandaloneExeComponentLoader() override;

    QObject* load(const ComponentManifest& manifest, const QString& basePath) override;
    bool unload(QObject* component) override;
    bool canLoad(const ComponentManifest& manifest, const QString& basePath) const override;
    ComponentType supportedType() const override { return ComponentType_StandaloneExe; }

private:
    QString resolveExecutablePath(const ComponentManifest& manifest, const QString& basePath) const;
    QMap<QString, StandaloneExeWrapper*> m_runningProcesses;
};

#endif // STANDALONE_EXE_COMPONENT_LOADER_H
