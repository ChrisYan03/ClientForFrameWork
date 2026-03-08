#ifndef PICMATCHWIDGET_H
#define PICMATCHWIDGET_H

#include "PicPlayerDataDef.h"
#include "BaseWidget.h"
#include "FaceShowWidget.h"
#include <QPushButton>
#include <QLabel>
#include <QToolButton>
#include <QLineEdit>
#include <QFrame>
#include <QVariantMap>
#include <map>
#include <memory>

class QTimer;
class QWidget;
class QScrollArea;
class QStackedWidget;
class PicMatchViewModel;

/**
 * @brief PicMatch QWidget组件
 *
 * 渐进式MVVM迁移：
 * - UI层：保持QWidget实现
 * - 业务逻辑：委托给PicMatchViewModel
 */
class PicMatchWidget : public BaseWidget
{
    Q_OBJECT

public:
    PicMatchWidget(BaseWidget *parent = nullptr);
    ~PicMatchWidget();

    void InitUI();
    void InitPicPlayer();

    void Run();
    void Quit();

    /** 根据主框架主题色更新右侧面板等控件样式（与主框架换肤一致） */
    void applyTheme(QVariantMap themeColors);

    /** 当前数据/图片根路径：若用户配置过则返回配置路径，否则返回默认 componentDataPath */
    QString getDataPath() const;

    /** 获取ViewModel（供外部访问） */
    PicMatchViewModel* viewModel() const { return m_viewModel; }

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    static void* PicCallbackByPlayer(int handle, int iMsg, void* pData, void* pUser);
    void OnRun(const std::string& showid);
    void UpdatePic(const std::string& showid, const std::string& imagePath);
    /** 仅在收到 Callback_ShowPicId 时调用：根据 showid 从缓存取人脸结果并展示到人脸区 */
    void showFacesForShowId(const std::string& showid);
    bool LoadJpegToRGBA(const char* imagePath, PicShowInfo* demodata);
    void RotateImage90Degrees(unsigned char* imageData, unsigned char*& rotatedData, int width, int height, int channels);
    std::string GetNextImageName();
    void UpdateUIState();
    void CreateButtonBar(QWidget* parent);
    void CreateConfigPanel(QStackedWidget* stack);
    void updateButtonBarStyle();
    void loadDataPath();
    void saveDataPath();

    // ViewModel信号处理
    void setupViewModelConnections();

private slots:
    void OnRunButtonClicked();
    void OnStopButtonClicked();
    void OnConfigButtonClicked();
    void OnConfigApply();
    void OnConfigBrowse();

    // ViewModel信号响应
    void onViewModelRunningChanged(bool running);
    void onViewModelImageUpdated(const QString& showId, const QString& imagePath);
    void onViewModelFaceListChanged();

private:
    int m_handle;
    BaseWidget * m_playerWidget;
    FaceShowWidget* m_faceShowWidget;
    std::string m_showId;
    std::vector<std::string> m_imageNames;
    size_t m_currentIndex;
    bool m_initialized;
    bool m_running;

    /** 用户配置的图片查看路径，为空时使用默认 componentDataPath */
    QString m_customDataPath;

    /** 按 showId 缓存人脸检测结果，收到 Callback_ShowPicId 时再取缓存展示人脸区（最多保留 3 条） */
    std::map<std::string, std::unique_ptr<FaceDetectionResult>> m_faceResultCache;

    /** 为 true 时 onViewModelFaceListChanged 才刷新人脸区，仅由 showFacesForShowId 置位，避免 ViewModel 其他路径（如内部 Run）一 emit faceListChanged 就刷脸 */

    // 右侧面板：VS Code 风格图标栏 + 人脸区
    QWidget* m_rightPanel;
    QFrame* m_buttonBarWidget;
    QToolButton* m_runButton;
    QToolButton* m_stopButton;
    QToolButton* m_configButton;

    QStackedWidget* m_rightStack;
    QWidget* m_configPanel;
    QLineEdit* m_configPathEdit;

    /** 上次应用的主题色，用于运行状态变化时刷新按钮栏样式（运行中启动按钮置绿） */
    QVariantMap m_lastThemeColors;

    // ViewModel（管理业务逻辑）
    PicMatchViewModel* m_viewModel;

#if defined(Q_OS_WIN)
    QTimer *m_resizeNotifyTimer = nullptr;
#endif
};

#endif // PICMATCHWIDGET_H
