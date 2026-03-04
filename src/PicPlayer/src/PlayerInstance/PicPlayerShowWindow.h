#ifndef PICPLAYERSHOWWINDOW_H
#define PICPLAYERSHOWWINDOW_H

#ifdef _WIN32
    #include <Windows.h>
#endif
#include <atomic>
#include "glew.h"
#include "PicPlayerGui.h"

struct GLFWwindow;
class PicPlayerShowWindow : public PicPlayerGui
{
public:
    static void WindowSizeCallback(GLFWwindow* window, int width, int height);
    /** 设置 OpenGL/ImGui 主题背景色（0~1），由宿主在换肤时调用 */
    static void SetThemeBackgroundColor(float r, float g, float b);

public:
    PicPlayerShowWindow(Window_ShowID hParent, int iCacheNum = 10);
    ~PicPlayerShowWindow();

    virtual void Destroy() override;
    virtual int RunRendLoop() override;
    virtual void Quit() override;

    void OnResize(int width, int height);
    /** 由宿主（Qt）在嵌入区域 resize 时调用，下一帧渲染时会应用并更新视口 */
    void SetDesiredSize(int width, int height);

protected:
    bool CreateRenderWindow();
    void Draw();
    void Render();
    void DestroyRenderWindow();

private:
    static void ApplyThemeClearColor();
    void checkAndApplyResize();

    GLFWwindow* m_window;
    int m_lastViewportWidth = 0;
    int m_lastViewportHeight = 0;
    std::atomic<int> m_desiredWidth{0};
    std::atomic<int> m_desiredHeight{0};
    int m_iCacheNum;
    #ifdef _WIN32
    HWND m_hParent;
    #else
        Window_ShowID m_hParent;
    #endif
};

#endif // PICPLAYERSHOWWINDOW_H

