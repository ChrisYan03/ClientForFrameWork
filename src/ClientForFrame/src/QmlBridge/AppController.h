#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <QObject>

class PicMatchWidget;
class QWidget;

/**
 * @brief 供 QML 调用的 C++ 桥接类，暴露开始/停止/关闭及状态。
 * 可在此处接入原有 PicMatchWidget 逻辑（Run/Quit 等）。
 */
class AppController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)

public:
    explicit AppController(QObject *parent = nullptr);
    ~AppController();

    QString statusText() const { return m_statusText; }

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void closeApp();

    /** 由 main_qml 注入播放器窗口与 PicMatchWidget，用于 Run/Quit */
    void setPlayer(PicMatchWidget *widget, QWidget *hostWindow);

signals:
    void statusTextChanged();
    /** 请求退出应用，由 main 连接 QApplication::quit */
    void requestQuit();

private:
    void setStatusText(const QString &text);

    QString m_statusText;
    // 可选：保留原有业务组件引用，在 start/stop 中调用
    QWidget *m_hostWindow;
    PicMatchWidget *m_picMatchWidget;
};

#endif // APPCONTROLLER_H
