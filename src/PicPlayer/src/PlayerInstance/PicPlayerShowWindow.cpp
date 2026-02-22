#include "PicPlayerShowWindow.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "glfw3.h"
#include "../Render/PicPlayerRender.h"
#ifdef __APPLE__
    #include "PicPlayerWindowForMac.h"
    #include <dispatch/dispatch.h>
    #include <unistd.h>  // for usleep
#else
    #define GLFW_EXPOSE_NATIVE_WIN32
    #include <GLFW/glfw3native.h>
#endif

#include "../Render/DrawPicByImgui/PicTexture.h"
#include "LogUtil.h"

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
    LOG_DEBUG("Destroy");
    DestroyRenderWindow();
}

int PicPlayerShowWindow::RunRendLoop()
{
    if (m_window == nullptr) {
        bool success = CreateRenderWindow();
        if (!success){
            LOG_ERROR("Failed to create render window");
            return -1;
        }
    }

    // 在 macOS 上，我们需要不同的处理方式
#ifdef __APPLE__
    // 使用标志位控制循环，避免依赖 glfwWindowShouldClose
    bool shouldContinue = true;
    while (shouldContinue && m_window) {
        // 检查外部停止请求
        if (glfwWindowShouldClose(m_window)) {
            LOG_DEBUG("Window should close detected");
            shouldContinue = false;
            break;
        }
        
        glClear(GL_COLOR_BUFFER_BIT);
        GetRender()->InitFramerate(ImGui::GetIO().Framerate);
        glfwPollEvents();
        Draw();
        Render();
        
        // 再次检查窗口状态
        if (m_window && !glfwWindowShouldClose(m_window)) {
            glfwSwapBuffers(m_window);
        } else {
            LOG_DEBUG("Window closed during buffer swap");
            shouldContinue = false;
            break;
        }
    }
#else
    // Windows/Linux 版本保持原样
    while (m_window && !glfwWindowShouldClose(m_window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        GetRender()->InitFramerate(ImGui::GetIO().Framerate);
        glfwPollEvents();
        Draw();
        Render();
        glfwSwapBuffers(m_window);
    }
#endif

    LOG_DEBUG("Exit render loop, destroying window");
    if (m_window) {
        DestroyRenderWindow();
    }

    LOG_DEBUG("Render loop finished completely");
    return 0;
}

void PicPlayerShowWindow::Quit()
{
#ifdef __APPLE__
    // 不使用 dispatch_async，而是直接在当前线程处理
    LOG_DEBUG("Quit called on window: {}", (void*)m_window);
    
    if (m_window) {
        // 直接设置关闭标志
        glfwSetWindowShouldClose(m_window, 1);
        
        // 强制唤醒事件循环
        glfwPostEmptyEvent();
        
        // 给一点时间让事件循环处理
        usleep(10000); // 10ms
        
        // 如果窗口仍未关闭，强制销毁
        if (m_window) {
            LOG_DEBUG("Force destroying window");
            glfwDestroyWindow(m_window);
            m_window = nullptr;
        }
    }
    LOG_DEBUG("Quit completed");
#else
    if (m_window) {
        glfwSetWindowShouldClose(m_window, 1);
    }
#endif
}

void PicPlayerShowWindow::OnResize(int width, int height)
{
    LOG_DEBUG("OnResize: {} x {}", width, height);
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
    // 添加错误回调以便更好地诊断问题
    glfwSetErrorCallback([](int error, const char* description) {
        LOG_ERROR("GLFW Error {}: {}", error, description);
    });
#ifdef _WIN32
    if (m_hParent != NULL) {
        RECT rect;
        GetWindowRect(m_hParent, &rect);
        LOG_DEBUG("Window size: {} x {}", rect.right - rect.left, rect.bottom - rect.top);
        m_window = glfwCreateWindow(rect.right - rect.left, rect.bottom - rect.top, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
        if (m_window == nullptr) {
            LOG_ERROR("Failed to create GLFW window");
            return false;
        }
        SetParent(glfwGetWin32Window(m_window), m_hParent);
    } else {
        glfwDefaultWindowHints();
        m_window = glfwCreateWindow(1280, 720, "ImGui PicPlayer", nullptr, nullptr);
        if (m_window == nullptr) {
            LOG_ERROR("Failed to create GLFW window");
            return false;
        }
    }
#elif __APPLE__
    if (m_hParent != 0) {
        int width = 0, height = 0;
        if (!GetWindowSizeForMac((void*)m_hParent, width, height)) {
            LOG_ERROR("Failed GetWindowSizeForMac");
            return false;
        }
        m_window = glfwCreateWindow(width, height, "ImGui PicPlayer", nullptr, nullptr);
        if (m_window) {
            if (!SetChildWindow((void*)m_hParent, m_window)) {
                LOG_ERROR("Failed SetChildWindow");
                glfwDestroyWindow(m_window);
                m_window = nullptr;
                return false;
            }
        }
        else{
            LOG_ERROR("Failed to create GLFW window for Mac child window");
            return false;
        }
    }
#endif

    // 确保窗口创建成功后再进行后续初始化
    if (!m_window) {
        LOG_ERROR("Window creation failed - window is null");
        return false;
    }
    GetRender()->GetSynchronizer()->SetEnable(true);
    glfwSetWindowUserPointer(m_window, this);
    glfwSetWindowSizeCallback(m_window, PicPlayerShowWindow::WindowSizeCallback);
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1); // 鍚敤鍨傜洿鍚屾

    if (glewInit() != GLEW_OK) {
        LOG_ERROR("Failed to initialize GLEW");
        glfwDestroyWindow(m_window);
        m_window = nullptr;
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
    imguiStyle.Colors[ImGuiCol_WindowBg] = ImVec4(0.25f, 0.25f, 0.25f, 0.8f);imguiStyle.WindowPadding = ImVec2(0, 0);
    imguiStyle.WindowBorderSize = 0.0;
    imguiStyle.DisplayWindowPadding = ImVec2(0, 0);
    imguiStyle.DisplaySafeAreaPadding = ImVec2(0, 0);

    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    const char* glslVersion = "#version 150";
    ImGui_ImplOpenGL3_Init(glslVersion);

    PicTexture::instance()->InitPicTexturePool(m_iCacheNum);

    int width, height;
    glfwGetWindowSize(m_window, &width, &height);
    LOG_DEBUG("glfwGetWindowSize size: {} x {}", width, height);
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
    if (nullptr == m_window || glfwWindowShouldClose(m_window)) {
        return;
    }
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void PicPlayerShowWindow::DestroyRenderWindow()
{
    LOG_DEBUG("DestroyRenderWindow start");
     // 首先确保不再进行渲染
     if (m_window) {
        glfwMakeContextCurrent(m_window);
    }
    // 清理渲染缓存（在ImGui上下文仍有效时）
    if (GetRender()) {
        GetRender()->ClearRenderCache();
        GetRender()->GetSynchronizer()->SetEnable(false);
    }
    // 按正确顺序释放ImGui资源
    if (m_window) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
    // 释放GLFW资源
    if (m_window) {
        // 强制设置窗口关闭标志
        glfwSetWindowShouldClose(m_window, 1);
        glfwDestroyWindow(m_window);
        LOG_DEBUG("glfwSetWindowShouldClose");
        m_window = nullptr;
    }
    
    LOG_DEBUG("DestroyRenderWindow end");
}

