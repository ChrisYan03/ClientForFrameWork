#include "PicPlayerShowWindow.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "glfw3.h"
#include "../Render/PicPlayerRender.h"
#ifdef __APPLE__
#include "PicPlayerWindowForMac.h"
#include <dispatch/dispatch.h>
#else
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

#include "../Render/DrawPicByImgui/PicTexture.h"
#include <iostream>

void PicPlayerShowWindow::WindowSizeCallback(GLFWwindow* window, int width, int height)
{
#ifdef __APPLE__
    dispatch_async(dispatch_get_main_queue(), ^{
        PicPlayerShowWindow* pThis = (PicPlayerShowWindow*)glfwGetWindowUserPointer(window);
        if (pThis){
            pThis->OnResize(width, height);
        }
    });
#else
    PicPlayerShowWindow* pThis = (PicPlayerShowWindow*)glfwGetWindowUserPointer(window);
    if (pThis) {
        pThis->OnResize(width, height);
    }
#endif
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
    DestroyRenderWindow();
}

int PicPlayerShowWindow::RunRendLoop()
{
    if (m_window == nullptr) {
        bool success = CreateRenderWindow();
        if (!success){
            std::cerr << "Failed GetWindowSizeForMac" << std::endl;
        }
    }

    while (!glfwWindowShouldClose(m_window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        GetRender()->InitFramerate(ImGui::GetIO().Framerate);
        glfwPollEvents();
        Draw();
        Render();
        glfwSwapBuffers(m_window);
    }
    DestroyRenderWindow();
    return 0;
}

void PicPlayerShowWindow::Quit()
{
#ifdef __APPLE__
    dispatch_async(dispatch_get_main_queue(), ^{
        if (m_window)
            glfwSetWindowShouldClose(m_window, 1);
    });
#else
    if (m_window)
        glfwSetWindowShouldClose(m_window, 1);
#endif
}

void PicPlayerShowWindow::OnResize(int width, int height)
{
    GetRender()->UpdateViewport(width, height);
}

bool PicPlayerShowWindow::CreateRenderWindow()
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // 娌℃湁杈规鍜屾爣棰樻爮
    glfwWindowHint(GLFW_DECORATED, GL_FALSE);
    glfwWindowHint(GLFW_AUTO_ICONIFY, GL_FALSE);
    // 鍚敤澶氶噸閲囨牱锛屾姉閿娇
    glfwWindowHint(GLFW_SAMPLES, 4);
#ifdef _WIN32
    if (m_hParent != NULL) {
        RECT rect;
        GetWindowRect(m_hParent, &rect);
        std::cout << rect.right - rect.left;
        m_window = glfwCreateWindow(rect.right - rect.left, rect.bottom - rect.top, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
        if (m_window == nullptr) {
            return 0;
        }
        SetParent(glfwGetWin32Window(m_window), m_hParent);
    } else {
        glfwDefaultWindowHints();
        m_window = glfwCreateWindow(1280, 720, "ImGui PicPlayer", nullptr, nullptr);
        if (m_window == nullptr) {
            return false;
        }
    }
#elif __APPLE__
    if (m_hParent != 0) {
        int width = 0, height = 0;
        if (!GetWindowSizeForMac((void*)m_hParent, width, height)) {
            std::cerr << "Failed GetWindowSizeForMac" << std::endl;
            return false;
        }
        m_window = glfwCreateWindow(width, height, "ImGui PicPlayer", nullptr, nullptr);
        if (m_window) {
            if (!SetChildWindow((void*)m_hParent, m_window)) {
                std::cerr << "Failed SetChildWindow" << std::endl;
                return false;
            }
        }
        else{
            return false;
        }
    }
#endif
    GetRender()->GetSynchronizer()->SetEnable(true);
    glfwSetWindowUserPointer(m_window, this);
    glfwSetWindowSizeCallback(m_window, PicPlayerShowWindow::WindowSizeCallback);
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1); // 鍚敤鍨傜洿鍚屾

    if (glewInit() != GLEW_OK) {
        return false;
    }
    glEnable(GL_MULTISAMPLE);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& imguiIo = ImGui::GetIO();
    imguiIo.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    imguiIo.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    imguiIo.IniFilename = NULL;

    ImGui::StyleColorsDark();
    ImGuiStyle& imguiStyle = ImGui::GetStyle();
    imguiStyle.WindowRounding = 0.0f;
    imguiStyle.Colors[ImGuiCol_WindowBg] = ImVec4(0.6, 0.2, 0.6, 0.4);
    imguiStyle.WindowPadding = ImVec2(0, 0);
    imguiStyle.WindowBorderSize = 0.0;
    imguiStyle.DisplayWindowPadding = ImVec2(0, 0);
    imguiStyle.DisplaySafeAreaPadding = ImVec2(0, 0);

    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    const char* glslVersion = "#version 150";
    ImGui_ImplOpenGL3_Init(glslVersion);

    PicTexture::instance()->InitPicTexturePool(m_iCacheNum);

    int width, height;
    glfwGetWindowSize(m_window, &width, &height);
    glfwSetWindowPos(m_window, 4, 6);
    glfwSetWindowSize(m_window, width - 4, height - 6);
    GetRender()->InitScene(ImRect(4, 6, width - 4, height - 6));

    return true;
}

void PicPlayerShowWindow::Draw()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    RenderScene();
}

void PicPlayerShowWindow::Render()
{
    if (nullptr == m_window) {
        return;
    }
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void PicPlayerShowWindow::DestroyRenderWindow()
{
    GetRender()->ClearRenderCache();
    GetRender()->GetSynchronizer()->SetEnable(false);
    if (m_window) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
}

