/**
 * @file EmbeddedExeComponentLoader.h
 * @brief 嵌入式可执行文件组件加载器
 */
#ifndef EMBEDDED_EXE_COMPONENT_LOADER_H
#define EMBEDDED_EXE_COMPONENT_LOADER_H

#include "../../IComponentLoader.h"
#include <QProcess>
#include <QString>

class EmbeddedExeWrapper : public QObject
{
    Q_OBJECT
public:
    explicit EmbeddedExeWrapper(const QString& executablePath, QObject* parent = nullptr);
    ~EmbeddedExeWrapper() override;

    bool start();
    bool stop();
    bool isRunning() const;
    QString getExecutablePath() const { return m_executablePath; }
    void setCEFBrowserView(void* view) { m_cefBrowserView = view; }
    void* getCEFBrowserView() const { return m_cefBrowserView; }

signals:
    void processStateChanged(bool running);
    void messageReceived(const QString& message);

private:
    QProcess* m_process;
    QString m_executablePath;
    void* m_cefBrowserView = nullptr;
};

class EmbeddedExeComponentLoader : public QObject, public IComponentLoader
{
    Q_OBJECT
public:
    explicit EmbeddedExeComponentLoader(QObject* parent = nullptr);
    ~EmbeddedExeComponentLoader() override;

    QObject* load(const ComponentManifest& manifest, const QString& basePath) override;
    bool unload(QObject* component) override;
    bool canLoad(const ComponentManifest& manifest, const QString& basePath) const override;
    ComponentType supportedType() const override { return ComponentType_EmbeddedExe; }

private:
    QString resolveExecutablePath(const ComponentManifest& manifest, const QString& basePath) const;
    QString extractUrl(const ComponentManifest& manifest) const;
    QMap<QString, EmbeddedExeWrapper*> m_loadedEmbeddedComponents;
};

#endif // EMBEDDED_EXE_COMPONENT_LOADER_H
