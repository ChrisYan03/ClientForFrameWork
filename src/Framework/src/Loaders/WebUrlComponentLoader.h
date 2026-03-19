/**
 * @file WebUrlComponentLoader.h
 * @brief Web URL组件加载器
 *
 * 支持加载基于CEF (Chromium Embedded Framework) 的Web组件
 */
#ifndef WEB_URL_COMPONENT_LOADER_H
#define WEB_URL_COMPONENT_LOADER_H

#include "../IComponentLoader.h"
#include <QString>

/**
 * @brief Web组件包装器
 *
 * 包装基于CEF的Web组件，提供与 IComponent 兼容的接口。
 * 加载指定的Web URL并显示在CEF浏览器中。
 */
class WebUrlWrapper : public QObject
{
    Q_OBJECT

public:
    explicit WebUrlWrapper(const QString& url, QObject* parent = nullptr);
    ~WebUrlWrapper() override;

    /**
     * @brief 加载Web页面
     * @return 是否成功加载
     */
    bool load();

    /**
     * @brief 获取URL
     * @return URL字符串
     */
    QString getUrl() const { return m_url; }

    /**
     * @brief 设置URL
     * @param url URL字符串
     */
    void setUrl(const QString& url);

    /**
     * @brief 获取CEF浏览器视图
     * @return 视图指针（具体类型取决于CEF集成）
     */
    void* getCEFBrowserView() const { return m_cefBrowserView; }

    /**
     * @brief 设置CEF浏览器视图
     * @param view CEF浏览器视图指针
     */
    void setCEFBrowserView(void* view) { m_cefBrowserView = view; }

signals:
    /**
     * @brief 页面加载完成信号
     */
    void pageLoaded();

    /**
     * @brief 页面加载失败信号
     * @param error 错误信息
     */
    void loadFailed(const QString& error);

private:
    QString m_url;
    void* m_cefBrowserView = nullptr; // CEF浏览器视图指针
};

/**
 * @brief Web URL组件加载器
 *
 * 负责加载基于CEF的Web组件。
 * 根据manifest中的URL配置创建Web组件包装器。
 */
class WebUrlComponentLoader : public QObject, public IComponentLoader
{
    Q_OBJECT

public:
    explicit WebUrlComponentLoader(QObject* parent = nullptr);
    ~WebUrlComponentLoader() override;

    // IComponentLoader 接口实现
    QObject* load(const ComponentManifest& manifest, const QString& basePath) override;
    bool unload(QObject* component) override;
    bool canLoad(const ComponentManifest& manifest, const QString& basePath) const override;
    ComponentType supportedType() const override { return ComponentType_WebUrl; }

private:
    /**
     * @brief 从manifest中提取URL
     * @param manifest 组件元数据
     * @return URL字符串
     */
    QString extractUrl(const ComponentManifest& manifest) const;

    /**
     * @brief 验证URL格式
     * @param url URL字符串
     * @return 是否有效
     */
    bool isValidUrl(const QString& url) const;

    // 管理已加载的Web组件
    QMap<QString, WebUrlWrapper*> m_loadedWebComponents;
};

#endif // WEB_URL_COMPONENT_LOADER_H