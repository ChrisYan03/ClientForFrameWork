#include "PicPlayerShowWindow.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "glfw3.h"
#include "../Render/PicPlayerRender.h"
#ifdef __APPLE__
#include "PicPlayerWindowForMac.h"
#include <dispatch/dispatch.h>
#endif
#include "../Render/DrawPicByImgui/PicTexture.h"
#include <iostream>

void PicPlayerShowWindow::WindowSizeCallback(GLFWwindow* window, int width, int height)
{
    dispatch_async(dispatch_get_main_queue(), ^{
        PicPlayerShowWindow* pThis = (PicPlayerShowWindow*)glfwGetWindowUserPointer(window);
        if (pThis){
            pThis->OnResize(width, height);
        }
    });
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
    // 分派任务到主线程创建 NSWindow
    dispatch_async(dispatch_get_main_queue(), ^{
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
    });
    DestroyRenderWindow();
    return 0;
}

void PicPlayerShowWindow::Quit()
{
    dispatch_async(dispatch_get_main_queue(), ^{
        if (m_window)
            glfwSetWindowShouldClose(m_window, 1);
    });
}

void PicPlayerShowWindow::OnResize(int width, int height)
{
    GetRender()->UpdateViewport(width, height);
}

bool PicPlayerShowWindow::CreateRenderWindow()
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
    else {
        glfwDefaultWindowHints();
        m_window = glfwCreateWindow(1280, 720, "ImGui PicPlayer", nullptr, nullptr);
        if (m_window == nullptr) {
            return false;
        }
    }
    GetRender()->GetSynchronizer()->SetEnable(true);
    glfwSetWindowUserPointer(m_window, this);
    glfwSetWindowSizeCallback(m_window, PicPlayerShowWindow::WindowSizeCallback);
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1); // 启用垂直同步

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
    dispatch_async(dispatch_get_main_queue(), ^{
        if (m_window) {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
            glfwDestroyWindow(m_window);
            m_window = nullptr;
        }
    });
}


// bool PicPlayerShowWindow::LoadImage(const char* imagePath)
// {
//     int imageChannels;
//     unsigned char* imageData = stbi_load(imagePath, &m_imageWidth, &m_imageHeight, &imageChannels, 0);
//     if (!imageData) {
//         std::cerr << "Failed to load image: " << imagePath << std::endl;
//         return false;
//     }

//     glGenTextures(1, &m_textureID);
//     glBindTexture(GL_TEXTURE_2D, m_textureID);

//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

//     GLenum format = imageChannels == 3 ? GL_RGB : GL_RGBA;
//     glTexImage2D(GL_TEXTURE_2D, 0, format, m_imageWidth, m_imageHeight, 0, format, GL_UNSIGNED_BYTE, imageData);

//     stbi_image_free(imageData);
//     glBindTexture(GL_TEXTURE_2D, 0);

//     return true;
// }
