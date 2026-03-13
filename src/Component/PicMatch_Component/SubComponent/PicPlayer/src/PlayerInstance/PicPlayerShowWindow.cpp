#include "PicPlayerShowWindow.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "glfw3.h"
#include "../Render/PicPlayerRender.h"
#include <mutex>
#ifdef __APPLE__
    #include "PicPlayerWindowForMac.h"
    #include <dispatch/dispatch.h>
    #include <unistd.h>  // for usleep
#else
    #define GLFW_EXPOSE_NATIVE_WIN32
    #include <GLFW/glfw3native.h>
#endif

#include "../Render/DrawPicByImgui/PicTexture.h"
#include "PicPlayerLog.h"

namespace {
    float s_themeBgR = 0.15f, s_themeBgG = 0.15f, s_themeBgB = 0.15f;
    std::mutex s_themeBgMutex;
}

void PicPlayerShowWindow::SetThemeBackgroundColor(float r, float g, float b)
{
    std::lock_guard<std::mutex> lock(s_themeBgMutex);
    s_themeBgR = r;
    s_themeBgG = g;
    s_themeBgB = b;
}

void PicPlayerShowWindow::ApplyThemeClearColor()
{
    float r, g, b;
    {
        std::lock_guard<std::mutex> lock(s_themeBgMutex);
        r = s_themeBgR;
        g = s_themeBgG;
        b = s_themeBgB;
    }
    glClearColor(r, g, b, 1.0f);
}

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
        
        ApplyThemeClearColor();
        glClear(GL_COLOR_BUFFER_BIT);
        GetRender()->InitFramerate(ImGui::GetIO().Framerate);
        glfwPollEvents();
        checkAndApplyResize();
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
        ApplyThemeClearColor();
        glClear(GL_COLOR_BUFFER_BIT);
        GetRender()->InitFramerate(ImGui::GetIO().Framerate);
        glfwPollEvents();
        checkAndApplyResize();
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
    LOG_DEBUG("Quit called on window: {}", (void*)m_window);
    if (m_window) {
        glfwSetWindowShouldClose(m_window, 1);
        glfwPostEmptyEvent();
        usleep(10000);
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
    // 添加错误回调以便更好地诊断问题；GLFW_NOT_INITIALIZED 多为退出/销毁顺序导致，不刷 error 日志
    glfwSetErrorCallback([](int error, const char* description) {
        if (error == GLFW_NOT_INITIALIZED)
            return;
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
    {
        std::lock_guard<std::mutex> lock(s_themeBgMutex);
        imguiStyle.Colors[ImGuiCol_WindowBg] = ImVec4(s_themeBgR, s_themeBgG, s_themeBgB, 0.95f);
    }
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
    LOG_DEBUG("glfwGetWindowSize size: {} x {}", width, height);
    glfwSetWindowPos(m_window, 0, 0);
    glfwSetWindowSize(m_window, width, height);
    m_lastViewportWidth = width;
    m_lastViewportHeight = height;
    GetRender()->InitScene(ImRect(0, 0, width, height));

    return true;
}

void PicPlayerShowWindow::SetDesiredSize(int width, int height)
{
    if (width > 0 && height > 0) {
        m_desiredWidth.store(width);
        m_desiredHeight.store(height);
    }
}

void PicPlayerShowWindow::checkAndApplyResize()
{
    if (!m_window)
        return;
    int w, h;
    int desiredW = m_desiredWidth.exchange(0);
    int desiredH = m_desiredHeight.exchange(0);
    if (desiredW > 0 && desiredH > 0) {
        w = desiredW;
        h = desiredH;
        glfwSetWindowSize(m_window, w, h);
    } else {
        w = 0;
        h = 0;
#ifdef _WIN32
        // 按帧节流：仅偶数帧读取父窗口尺寸并同步，减少 GetClientRect/glfwSetWindowSize 频率，最大化/恢复更顺滑
        const bool throttleFrame = (m_resizeThrottleFrame++ & 1u) != 0;
        if (m_hParent != NULL && !throttleFrame) {
            RECT rect;
            if (GetClientRect(m_hParent, &rect)) {
                w = rect.right - rect.left;
                h = rect.bottom - rect.top;
                if (w > 0 && h > 0 && (w != m_lastViewportWidth || h != m_lastViewportHeight))
                    glfwSetWindowSize(m_window, w, h);
            }
        }
#endif
#ifdef __APPLE__
        if (m_hParent && m_window && GetWindowSizeForMac((void*)m_hParent, w, h) && w > 0 && h > 0 && (w != m_lastViewportWidth || h != m_lastViewportHeight))
            glfwSetWindowSize(m_window, w, h);
#endif
        if (w < 1 || h < 1)
            glfwGetWindowSize(m_window, &w, &h);
    }
    if (w < 1) w = 1;
    if (h < 1) h = 1;
    if (w != m_lastViewportWidth || h != m_lastViewportHeight) {
        m_lastViewportWidth = w;
        m_lastViewportHeight = h;
        OnResize(w, h);
#ifdef __APPLE__
        // 子窗口方式嵌入时，主线程更新 NSWindow frame 以跟随父 view
        if (m_hParent && m_window) {
            void* parent = reinterpret_cast<void*>(m_hParent);
            GLFWwindow* win = m_window;
            dispatch_async(dispatch_get_main_queue(), ^{
                UpdateChildWindowFrameForMac(parent, win);
            });
        }
#endif
    }
}

void PicPlayerShowWindow::Draw()
{
    {
        std::lock_guard<std::mutex> lock(s_themeBgMutex);
        ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = ImVec4(s_themeBgR, s_themeBgG, s_themeBgB, 0.95f);
    }
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
#ifdef __APPLE__
        void* parent = reinterpret_cast<void*>(m_hParent);
        GLFWwindow* win = m_window;
        if (parent && win) {
            dispatch_sync(dispatch_get_main_queue(), ^{
                RemoveChildWindowForMac(parent, win);
            });
        }
#endif
        glfwSetWindowShouldClose(m_window, 1);
        glfwDestroyWindow(m_window);
        LOG_DEBUG("glfwSetWindowShouldClose");
        m_window = nullptr;
    }
    
    LOG_DEBUG("DestroyRenderWindow end");
}

