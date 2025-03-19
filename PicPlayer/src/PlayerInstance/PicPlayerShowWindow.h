#ifndef PICPLAYERSHOWWINDOW_H
#define PICPLAYERSHOWWINDOW_H

#ifdef _WIN32
#include <Windows.h>
#endif
#include "glew.h"
#include "PicPlayerGui.h"

struct GLFWwindow;
class PicPlayerShowWindow : public PicPlayerGui
{
public:
    static void WindowSizeCallback(GLFWwindow* window, int width, int height);

public:
    PicPlayerShowWindow(Window_ShowID hParent, int iCacheNum = 10);
    ~PicPlayerShowWindow();

    virtual void Destroy() override;
    virtual int RunRendLoop() override;
    virtual void Quit() override;

    void OnResize(int width, int height);

protected:
    bool CreateRenderWindow();
    void Draw();
    void Render();
    void DestroyRenderWindow();

private:
    GLFWwindow* m_window;
    int m_iCacheNum;
    #ifdef _WIN32
    HWND m_hParent;
    #else
    Window_ShowID m_hParent;
    #endif
};

#endif // PICPLAYERSHOWWINDOW_H
