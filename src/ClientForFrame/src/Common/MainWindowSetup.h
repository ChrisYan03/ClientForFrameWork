#ifndef MAINWINDOWSETUP_H
#define MAINWINDOWSETUP_H

class QQuickWindow;

/**
 * @brief 主窗口平台相关设置：圆角 mask、Win11 圆角、延迟 show 等。
 */
namespace MainWindowSetup
{
    /** 对主窗口应用圆角 mask、连接尺寸/可见性变化、执行 show（Windows 下延后 show 并设置 Win11 圆角） */
    void setup(QQuickWindow *window);
}

#endif // MAINWINDOWSETUP_H
