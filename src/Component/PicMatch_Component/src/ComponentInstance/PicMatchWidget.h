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

class QTimer;
class QWidget;
class QScrollArea;
class QStackedWidget;

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

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    static void* PicCallbackByPlayer(int handle, int iMsg, void* pData, void* pUser);
    void OnRun(const std::string& showid);
    void UpdatePic(const std::string& showid, const std::string& imagePath);
    bool LoadJpegToRGBA(const char* imagePath, PicShowInfo* demodata);
    void RotateImage90Degrees(unsigned char* imageData, unsigned char*& rotatedData, int width, int height, int channels);
    std::string GetNextImageName();
    void UpdateUIState();
    void CreateButtonBar(QWidget* parent);
    void CreateConfigPanel(QStackedWidget* stack);
    void updateButtonBarStyle();
    void loadDataPath();
    void saveDataPath();

private slots:
    void OnRunButtonClicked();
    void OnStopButtonClicked();
    void OnConfigButtonClicked();
    void OnConfigApply();
    void OnConfigBrowse();

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

#if defined(Q_OS_WIN)
    QTimer *m_resizeNotifyTimer = nullptr;
#endif
};

#endif // PICMATCHWIDGET_H
