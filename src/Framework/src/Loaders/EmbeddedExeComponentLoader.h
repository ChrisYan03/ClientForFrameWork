/**
 * @file EmbeddedExeComponentLoader.h
 * @brief 嵌入式可执行文件组件加载器
 *
 * 支持加载基于CEF嵌入的可执行文件组件
 */
#ifndef EMBEDDED_EXE_COMPONENT_LOADER_H
#define EMBEDDED_EXE_COMPONENT_LOADER_H

#include "../IComponentLoader.h"
#include <QProcess>
#include <QString>

/**
 * @brief 嵌入式可执行文件组件包装器
 *
 * 结合CEF和独立进程，提供���入式可执行文件的组件化包装。
 * 可以将外部可执行文件嵌入到CEF浏览器中运行。
 */
class EmbeddedExeWrapper : public QObject
{
    Q_OBJECT

public:
    explicit EmbeddedExeWrapper(const QString& executablePath, QObject* parent = nullptr);
    ~EmbeddedExeWrapper() override;

    /**
     * @brief 启动嵌入式进程
     * @return 是否成功启动
     */
    bool start();

    /**
     * @brief 停止嵌入式进程
     * @return 是否成功停止
     */
    bool stop();

    /**
     * @brief 检查进程是否运行中
     * @return 是否运行中
     */
    bool isRunning() const;

    /**
     * @brief 获取可执行文件路径
     * @return 可执行文件路径
     */
    QString getExecutablePath() const { return m_executablePath; }

    /**
     * @brief 设置CEF浏览器视图
     * @param view CEF浏览器视图��针
     */
    void setCEFBrowserView(void* view) { m_cefBrowserView = view; }

    /**
     * @brief 获取CEF浏览器视图
     * @return CEF浏览器视图指针
     */
    void* getCEFBrowserView() const { return m_cefBrowserView; }

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
    void* m_cefBrowserView = nullptr; // CEF浏览器视图指针
};

/**
 * @brief 嵌入式可执行文件组件加载器
 *
 * 负责加载和管理嵌入式可执行文件组件。
 * 结合了StandaloneExe和WebUrl的特点，既可以运行独立进程，
 * 又可以嵌入到CEF浏览器视图中。
 */
class EmbeddedExeComponentLoader : public QObject, public IComponentLoader
{
    Q_OBJECT

public:
    explicit EmbeddedExeComponentLoader(QObject* parent = nullptr);
    ~EmbeddedExeComponentLoader() override;

    // IComponentLoader 接口实现
    QObject* load(const ComponentManifest& manifest, const QString& basePath) override;
    bool unload(QObject* component) override;
    bool canLoad(const ComponentManifest& manifest, const QString& basePath) const override;
    ComponentType supportedType() const override { return ComponentType_EmbeddedExe; }

private:
    /**
     * @brief 解析可执行文件路径
     * @param manifest 组件元数据
     * @param basePath 基础路径
     * @return 可执行文件完整路径
     */
    QString resolveExecutablePath(const ComponentManifest& manifest, const QString& basePath) const;

    /**
     * @brief 从manifest中提取URL（用于CEF嵌入）
     * @param manifest 组件元数据
     * @return URL字符串
     */
    QString extractUrl(const ComponentManifest& manifest) const;

    // 管理已加载的嵌入式组件
    QMap<QString, EmbeddedExeWrapper*> m_loadedEmbeddedComponents;
};

#endif // EMBEDDED_EXE_COMPONENT_LOADER_H