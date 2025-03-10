#include "PicPlayerShowWindow.h"
#include "imgui_impl_glfw.h"
#include "glfw3.h"
#include "../Render/PicPlayerRender.h"

void PicPlayerShowWindow::WindowSizeCallback(GLFWwindow* window, int width, int height)
{
    PicPlayerShowWindow* pThis = (PicPlayerShowWindow*)glfwGetWindowUserPointer(window);
    if (pThis){
        pThis->OnResize(width, height);
    }
}

PicPlayerShowWindow::PicPlayerShowWindow(Window_ShowID hParent, int iCacheNum)
    :m_window(nullptr)
    ,m_iCacheNum(iCacheNum)
#ifdef _WIN32
    ,m_hParent((HWND)hParent)
#else
    ,m_hParent(hParent)
#endif
{

}

PicPlayerShowWindow::~PicPlayerShowWindow()
{

}

void PicPlayerShowWindow::Destroy()
{

}

int PicPlayerShowWindow::RunRendLoop()
{

    return 0;
}

void PicPlayerShowWindow::Quit()
{

}

void PicPlayerShowWindow::OnResize(int width, int height)
{

}

void PicPlayerShowWindow::CreateRenderWindow()
{

}

void PicPlayerShowWindow::Draw()
{

}

void PicPlayerShowWindow::Render()
{

}

void PicPlayerShowWindow::DestroyRenderWindow()
{

}
