/**
 * @file PicMatchComponent.h
 * @brief PicMatch 组件实现
 *
 * 继承 IComponent 接口，实现组件的标准化生命周期管理。
 * 组件信息通过 m_manifest 成员获取（由 Framework 自动从 manifest.json 解析）。
 */
#ifndef PICMATCHCOMPONENT_H
#define PICMATCHCOMPONENT_H

#include <QObject>
#include <QQmlEngine>
#include <QVariantMap>
#include "IComponent.h"
#include "IComponentData.h"

/**
 * @brief PicMatch 组件实现
 *
 * 继承 IComponent 接口。
 * 组件元信息（id/name/version/icon/qmlPage等）通过 m_manifest 获取，
 * 由 Framework 在加载时自动从 manifest.json 解析并填充。
 */
class PicMatchComponent : public QObject, public IComponent
{
    Q_OBJECT

public:
    explicit PicMatchComponent(QObject *parent = nullptr);
    ~PicMatchComponent() override;

    // ==================== IComponent 接口实现 ====================

    bool initialize(QQmlEngine *engine, const QString &basePath) override;
    void shutdown() override;
    void registerQmlTypes(QQmlEngine *engine) override;
    QObject* getInterface(const QString &interfaceName) override;

    // ==================== Framework 调用 ====================

    /// 由 Framework 调用，设置组件元信息
    Q_INVOKABLE void setManifest(const ComponentManifest &manifest) { m_manifest = manifest; }

    // ==================== 便捷访问 ====================

    /// 组件元信息（由 Framework 自动填充）
    ComponentManifest m_manifest;

    /// 检查是否已初始化
    bool isInitialized() const { return m_initialized; }

signals:
    void viewModelCreated(QObject *viewModel);

private:
    QQmlEngine *m_engine = nullptr;
    QString m_basePath;
    QObject *m_viewModel = nullptr;
    bool m_initialized = false;
};

#endif // PICMATCHCOMPONENT_H
