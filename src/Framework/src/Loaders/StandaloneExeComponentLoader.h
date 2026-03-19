/**
 * @file StandaloneExeComponentLoader.h
 * @brief 独立可执行文件���件加载器
 *
 * 支持加载和管理独立的 .exe (Windows) 和 .app (macOS) 组件
 */
#ifndef STANDALONE_EXE_COMPONENT_LOADER_H
#define STANDALONE_EXE_COMPONENT_LOADER_H

#include "../IComponentLoader.h"
#include <QProcess>
#include <QMap>

/**
 * @brief 独立可执行文件组件包装器
 *
 * 包装独立进程，提供与 IComponent 兼容���接口。
 * 通过进程间通信与外部可执行文件交互。
 */
class StandaloneExeWrapper : public QObject
{
    Q_OBJECT

public:
    explicit StandaloneExeWrapper(const QString& executablePath, QObject* parent = nullptr);
    ~StandaloneExeWrapper() override;

    /**
     * @brief 启动独立进程
     * @return 是否成功启动
     */
    bool start();

    /**
     * @brief 停止独立进程
     * @return 是否成功停止
     */
    bool stop();

    /**
     * @brief 检查进程是否运行中
     * @return 是否运行中
     */
    bool isRunning() const;

    /**
     * @brief 向进程发送消息
     * @param message 消息内容
     */
    void sendMessage(const QString& message);

signals:
    /**
     * @brief 进程状态变化信号
     * @param running 是否运行中
     */
    void processStateChanged(bool running);

    /**
     * @brief 收到进程消息信号
     * @param message 消息内容
     */
    void messageReceived(const QString& message);

private:
    QProcess* m_process;
    QString m_executablePath;
};

/**
 * @brief 独立可执行文件组件加载器
 *
 * 负责启动和管理独立进程形式的组件。
 * 通过 QProcess 启动外部可执行文件，并提供进程间通信能力。
 */
class StandaloneExeComponentLoader : public QObject, public IComponentLoader
{
    Q_OBJECT

public:
    explicit StandaloneExeComponentLoader(QObject* parent = nullptr);
    ~StandaloneExeComponentLoader() override;

    // IComponentLoader 接口实现
    QObject* load(const ComponentManifest& manifest, const QString& basePath) override;
    bool unload(QObject* component) override;
    bool canLoad(const ComponentManifest& manifest, const QString& basePath) const override;
    ComponentType supportedType() const override { return ComponentType_StandaloneExe; }

private:
    /**
     * @brief 解析可执行文件路径
     * @param manifest 组件元数据
     * @param basePath 基础路径
     * @return 可执行文件完整路径
     */
    QString resolveExecutablePath(const ComponentManifest& manifest, const QString& basePath) const;

    // 管理已启动的进程包装器
    QMap<QString, StandaloneExeWrapper*> m_runningProcesses;
};

#endif // STANDALONE_EXE_COMPONENT_LOADER_H