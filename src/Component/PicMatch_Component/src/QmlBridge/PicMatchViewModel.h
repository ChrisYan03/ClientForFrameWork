#ifndef PICMATCHVIEWMODEL_H
#define PICMATCHVIEWMODEL_H

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <QMap>
#include <QTimer>
#include "Core/PicMatchModel.h"

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

    // 人脸列表（供QML使用）
    Q_PROPERTY(QVariantList faceList READ faceList NOTIFY faceListChanged)

    // 引擎句柄
    Q_PROPERTY(int playerHandle READ playerHandle NOTIFY handleChanged)

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

    // 引擎句柄
    int playerHandle() const { return m_playerHandle; }

    // QML调用接口
    Q_INVOKABLE void run();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void loadConfig();
    Q_INVOKABLE void saveConfig();
    Q_INVOKABLE void browseDataPath();

    // 主题应用
    Q_INVOKABLE void applyTheme(const QVariantMap& themeColors);

    // 初始化
    void initialize();
    void shutdown();

    // 注册窗口句柄（由QML调用）
    Q_INVOKABLE void registerWindow(void* windowId);

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

private slots:
    void onImageUpdated(const QString& showId, const QString& imagePath);

private:
    void updateUIState();
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
    PicMatchModel* m_model;
    /** 按 showId 缓存人脸列表，收到 Callback_ShowPicId 时恢复并 emit faceListChanged，保证「完全展示后才刷脸」 */
    QMap<QString, QList<FaceData>> m_faceCacheByShowId;
    bool m_running;
    QString m_statusText;
    QVariantMap m_themeColors;
    QVariantMap m_lastThemeColors;
    int m_playerHandle;

    // Windows resize timer
#if defined(Q_OS_WIN)
    QTimer* m_resizeNotifyTimer;
#endif
};

#endif // PICMATCHVIEWMODEL_H
