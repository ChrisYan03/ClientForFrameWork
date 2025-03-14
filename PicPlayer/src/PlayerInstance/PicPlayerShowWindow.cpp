#include "PicPlayerShowWindow.h"
#include "glew.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "glfw3.h"
#include "../Render/PicPlayerRender.h"
#ifdef __APPLE__
#include "PicPlayerWindowForMac.h"
#endif
#include <iostream>

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
    DestroyRenderWindow();
}

int PicPlayerShowWindow::RunRendLoop()
{
    if (m_window == nullptr) {
        CreateRenderWindow();
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
    if (m_window)
        glfwSetWindowShouldClose(m_window, 1);
}

void PicPlayerShowWindow::OnResize(int width, int height)
{
    //GetRender()->UpdateViewport(width, height);
}

void PicPlayerShowWindow::CreateRenderWindow()
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // 没有边框和标题栏
    glfwWindowHint(GLFW_DECORATED, GL_FALSE);
    glfwWindowHint(GLFW_AUTO_ICONIFY, GL_FALSE);
    // 启用多重采样，抗锯齿
    glfwWindowHint(GLFW_SAMPLES, 4);
#ifdef _WIN32
    if (m_hParent != NULL) {

    }
#elif __APPLE__
    if (m_hParent != 0) {
        int width = 0, height = 0;
        if (!GetWindowSizeForMac((void*)m_hParent, width, height)) {
            std::cerr << "Failed GetWindowSizeForMac" << std::endl;
            return;
        }
        m_window = glfwCreateWindow(width, height, "ImGui PicPlayer", nullptr, nullptr);
        if (m_window) {
            if (!SetChildWindow((void*)m_hParent, m_window)) {
                std::cerr << "Failed SetChildWindow" << std::endl;
                return;
            }
        }
        else{
            return;
        }
    }
#endif
    else {
        glfwDefaultWindowHints();
        m_window = glfwCreateWindow(1280, 720, "ImGui PicPlayer", nullptr, nullptr);
        if (m_window == nullptr) {
            return;
        }
    }
    //GetRender()->GetSynchronizer()->SetEnable(true);
    glfwSetWindowUserPointer(m_window, this);
    glfwSetWindowSizeCallback(m_window, PicPlayerShowWindow::WindowSizeCallback);
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1); // 启用垂直同步

    if (glewInit() != GLEW_OK) {
        return;
    }
    glEnable(GL_MULTISAMPLE);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& imguiIo = ImGui::GetIO();
    imguiIo.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    imguiIo.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    imguiIo.IniFilename = NULL;

    //ImGui::StyleColorsDark();
    ImGuiStyle& imguiStyle = ImGui::GetStyle();
    imguiStyle.WindowRounding = 0.0f;
    imguiStyle.Colors[ImGuiCol_WindowBg] = ImVec4(0.2, 0.8, 0.2, 1.0);
    imguiStyle.WindowPadding = ImVec2(0, 0);
    imguiStyle.WindowBorderSize = 0.0;
    imguiStyle.DisplayWindowPadding = ImVec2(0, 0);
    imguiStyle.DisplaySafeAreaPadding = ImVec2(0, 0);

    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    const char* glslVersion = "#version 150";
    ImGui_ImplOpenGL3_Init(glslVersion);
    ImGui::StyleColorsLight();

    /*int width, height;
    glfwGetWindowSize(m_window, &width, &height);
    glfwSetWindowPos(m_window, 4, 6);
    glfwSetWindowSize(m_window, width - 4, height - 6);
    GetRender()->InitScene(ImRect(4, 6, width - 4, height - 6));*/
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
