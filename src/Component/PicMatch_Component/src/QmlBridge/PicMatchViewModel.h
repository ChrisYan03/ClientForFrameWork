#ifndef PICMATCHVIEWMODEL_H
#define PICMATCHVIEWMODEL_H

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <QMap>
#include <QTimer>
#include <QPointer>
#include <QWindow>
#include "Core/PicMatchModel.h"
#include "FaceListModel.h"

/**
 * @brief PicMatch视图模型
 * 负责业务逻辑处理，作为QML与后端的桥梁
 *
 * 职责：
 * - 运行状态管理 (run/stop)
 * - 配置管理 (loadConfig/saveConfig)
 * - 主题管理 (applyTheme)
 * - 信号转发 (imageUpdated, faceListChanged等)
 */
class PicMatchViewModel : public QObject
{
    Q_OBJECT

    // 运行状态
    Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)

    // 数据路径
    Q_PROPERTY(QString dataPath READ dataPath NOTIFY dataPathChanged)

    // 人脸列表（供QML使用，兼容旧代码）
    Q_PROPERTY(QVariantList faceList READ faceList NOTIFY faceListChanged)
    // 人脸列表模型（供 QML 使用命名 role，通过 imageProviderUrl 取图）
    Q_PROPERTY(FaceListModel* faceModel READ faceModel NOTIFY faceListChanged)

    // 引擎句柄
    Q_PROPERTY(int playerHandle READ playerHandle NOTIFY handleChanged)

    /** 组件自身皮肤（从 resource/themes/light.json 或 dark.json 加载），QML 优先使用此项 */
    Q_PROPERTY(QVariantMap componentThemeColors READ componentThemeColors NOTIFY componentThemeColorsChanged)

public:
    explicit PicMatchViewModel(QObject* parent = nullptr);
    ~PicMatchViewModel();

    // 运行状态
    bool isRunning() const { return m_running; }
    QString statusText() const { return m_statusText; }

    // 数据路径
    QString dataPath() const { return m_model->getCurrentDataPath(); }

    // 人脸列表
    QVariantList faceList() const { return m_model->facesToVariantList(); }
    FaceListModel* faceModel() const { return m_faceListModel; }

    // 引擎句柄
    int playerHandle() const { return m_playerHandle; }

    /** 组件皮肤：theme 0=light, 1=dark，从组件 resource/themes/*.json 加载并更新 componentThemeColors */
    QVariantMap componentThemeColors() const { return m_componentThemeColors; }
    Q_INVOKABLE void setComponentTheme(int theme);

    // QML调用接口
    Q_INVOKABLE void run();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void loadConfig();
    Q_INVOKABLE void saveConfig();
    Q_INVOKABLE void browseDataPath();

    // 主题应用
    Q_INVOKABLE void applyTheme(const QVariantMap& themeColors);

    // 初始化（QML 可调用）
    Q_INVOKABLE void initialize();
    Q_INVOKABLE void shutdown();

    // 注册窗口句柄（由QML调用）。可传 QWindow* 或 void* 原生 id
    Q_INVOKABLE void registerWindow(QObject* windowObject);
    /** 播放区尺寸变化时由 PlayerHostItem 调用 */
    Q_INVOKABLE void setPlayerWindowSize(int width, int height);

    // 供外部调用的回调处理
    void onPlayerCallback(int handle, int msg, void* data);

    // 设置数据路径（供QML绑定）
    void setDataPath(const QString& path);

    /** 从人脸检测结果同步到 Model 并发出 faceListChanged，供 Widget 的 UpdatePic 路径统一走 onViewModelFaceListChanged */
    void setFacesFromDetectionResult(void* faceResult, const QString& showId);

signals:
    // 状态变化信号
    void runningChanged(bool running);
    void statusTextChanged(const QString& text);
    void dataPathChanged(const QString& path);
    void faceListChanged();
    void handleChanged(int handle);

    // 图片更新信号（通知渲染器）
    void imageUpdated(const QString& showId, const QString& imagePath);

    // 配置面板相关
    void configApplied();

    void componentThemeColorsChanged();

private slots:
    void onImageUpdated(const QString& showId, const QString& imagePath);

private:
    void updateUIState();
    void publishFaceList(const QVariantList& list);
    void loadComponentTheme(int theme);
    QString getEffectiveBaseDir() const;
    QString getComponentDataPath() const;

    // PicPlayer/PicRecognition API
    void initPicPlayer();
    void destroyPicPlayer();
    void initFaceRecognition(const QString& binPath);
    void destroyFaceRecognition();

    // 静态回调处理
    static void* picCallbackByPlayer(int handle, int msg, void* data, void* user);

private:
    void doRegisterWindow(QWindow* w);

    PicMatchModel* m_model;
    FaceListModel* m_faceListModel;
    /** 按 showId 缓存人脸列表，收到 Callback_ShowPicId 时恢复并 emit faceListChanged，保证「完全展示后才刷脸」 */
    QMap<QString, QList<FaceData>> m_faceCacheByShowId;
    /** 播放区宿主窗口，可在 run() 前由 QML registerWindow 传入，run() 时再注册到 PicPlayer */
    QPointer<QWindow> m_hostWindowForPlayer;
    bool m_running;
    QString m_statusText;
    QVariantMap m_themeColors;
    QVariantMap m_lastThemeColors;
    QVariantMap m_componentThemeColors;
    int m_playerHandle;

    // Windows resize timer
#if defined(Q_OS_WIN)
    QTimer* m_resizeNotifyTimer;
#endif
};

#endif // PICMATCHVIEWMODEL_H
