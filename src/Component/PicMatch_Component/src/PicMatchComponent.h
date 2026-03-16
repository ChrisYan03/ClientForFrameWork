/**
 * @file PicMatchComponent.h
 * @brief PicMatch 组件实现
 *
 * 提供统一的组件接口封装，支持主框架通过 Framework C API 管理组件生命周期。
 */
#ifndef PICMATCHCOMPONENT_H
#define PICMATCHCOMPONENT_H

#include <QObject>
#include <QString>
#include <QQmlEngine>
#include <QVariantMap>
#include <IComponentData.h>

// 前向声明
class PicMatchViewModel;
class PlayerHostItem;
class FaceImageProvider;

/**
 * @brief PicMatch 组件实现
 *
 * 封装现有的 PicMatchViewModel，配合 Framework C API 使用。
 * 组件通过 createComponent() 工厂函数创建，供 FrameworkComponentLoader 使用。
 */
class PicMatchComponent : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit PicMatchComponent(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~PicMatchComponent() override;

    // ==================== 组件接口实现 ====================

    QString id() const;
    QString name() const;
    QString version() const;
    QString description() const;
    QString author() const;
    QString iconPath() const;
    QString qmlPage() const;
    ComponentType type() const;
    ComponentState state() const;
    bool isInitialized() const;

    bool initialize(QQmlEngine *engine, const QString &basePath);
    void shutdown();
    Q_INVOKABLE void registerQmlTypes(QQmlEngine *engine);
    QObject* getInterface(const QString &interfaceName);

    // ==================== 额外接口 ====================

    /**
     * @brief 获取 ViewModel 实例
     * @return PicMatchViewModel 指针
     */
    QObject* getViewModel() const;

    /**
     * @brief 获取 PlayerHostItem 类名（用于 QML 类型注册）
     * @return 类名字符串
     */
    static QString playerHostItemClassName();

    /**
     * @brief 获取 ViewModel 类名（用于 QML 类型注册）
     * @return 类名字符串
     */
    static QString viewModelClassName();

    /**
     * @brief 设置组件状态
     * @param state 新状态
     */
    void setState(ComponentState state);

signals:
    /**
     * @brief ViewModel 已创建信号
     * @param viewModel ViewModel 实例
     */
    void viewModelCreated(QObject *viewModel);

private:
    /**
     * @brief 注册 QML 类型
     */
    void registerQmlTypesInternal(QQmlEngine *engine);

    /**
     * @brief 从 manifest.json 加载元信息
     * @param basePath 组件基础路径
     * @return 是否成功
     */
    bool loadMetaInfo(const QString &basePath);

private:
    QQmlEngine *m_engine = nullptr;
    QString m_basePath;
    QObject *m_viewModel = nullptr;
    QObject *m_playerHostItem = nullptr;
    bool m_qmlTypesRegistered = false;

    // 组件元信息（从 manifest.json 读取）
    QString m_id;
    QString m_name;
    QString m_version;
    QString m_description;
    QString m_author;
    QString m_iconPath;
    QString m_qmlPage;
    ComponentType m_type = ComponentType_NativeDll;
    ComponentState m_state = ComponentState_Loaded;
};

#endif // PICMATCHCOMPONENT_H
