#include "PicPlayer.h"
#include "PicPlayerVideoRender.h"
#include "PicPlayerCtrlDelegate.h"
#include <iostream>
#include <chrono>
#include "PicPlayerLog.h"
#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#include <pthread.h>
#endif

PicPlayer::PicPlayer(int cacheNum)
    : m_cacheNum(cacheNum)
    , m_handle(-1)
    , m_bStop(true)
    , m_wid(0)
    , m_guiPtr(nullptr)
    , m_ctrlDelPtr(nullptr)
    , m_renderPtr(nullptr)
{
    StartControllerThread();
}

PicPlayer::~PicPlayer()
{
    LOG_DEBUG("~PicPlayer start");
    StopControllerThread();
    StopPlayer();
    LOG_DEBUG("~PicPlayer suc");
}

void PicPlayer::SetHandle(int handle)
{
    m_handle = handle;
}

void PicPlayer::SetPicCallback(int handle, PlayerMsgCallback callback, void* pUser)
{
    if (m_ctrlDelPtr) {
        m_ctrlDelPtr->SetPicCallbackByDelegate(handle, callback, pUser);
    }
}

bool PicPlayer::InputPicData(int type, void* showData)
{
    if (m_bStop) {
        return false;
    }
    if (showData == nullptr) {
        return false;
    }
    if (m_ctrlDelPtr) {
        m_ctrlDelPtr->InputPicData(type, showData);
    }
    return true;
}

bool PicPlayer::StartPlayer()
{
    if (!m_bStop)
        return false;
    m_bStop = false;

    m_renderPtr = std::make_shared<PicPlayerVideoRender>(m_cacheNum);
    if (m_ctrlDelPtr) {
        m_ctrlDelPtr->SetRenderSync(m_renderPtr->GetSynchronizer());
    }
    m_renderThreadExited.store(false);
    m_tRenderThread = std::thread(std::bind(&PicPlayer::RenderThreadProc, this));
    auto thread = m_tRenderThread.native_handle();
    #ifdef __APPLE__
        //璁剧疆绾跨▼浼樺厛绾?
        sched_param param;
        param.sched_priority = 99;
        if (0 != pthread_setschedparam(thread, SCHED_FIFO, &param)) {
            std::cerr << "pthread_setschedparam Failed" << std::endl;
        }

    #endif
    return true;
}

bool PicPlayer::StopPlayer()
{
    m_bStop = true;
    if (m_guiPtr) {
        m_guiPtr->Quit();
    }
    if (m_tRenderThread.joinable()) {
#ifdef __APPLE__
        // 若在主线程直接 join，渲染线程里的 dispatch_sync(main) 会互锁。
        if (pthread_main_np() != 0) {
            const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
            while (!m_renderThreadExited.load() && std::chrono::steady_clock::now() < deadline) {
                CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.005, true);
            }
        }
#endif
        m_tRenderThread.join();
    }
    if (m_ctrlDelPtr) {
        m_ctrlDelPtr->SetRenderSync(nullptr);
    }
    if (m_renderPtr) {
        m_renderPtr.reset();
    }
    return true;
}

bool PicPlayer::InputFaceRecogResult(void* recogResult)
{
    if (m_bStop) {
        return false;
    }
    if (recogResult == nullptr) {
        return false;
    }
    if (m_ctrlDelPtr) {
        m_ctrlDelPtr->InputFaceRecogResult(recogResult);
    }
    return true;
}

PicPlayerRender* PicPlayer::GetRender() const
{
    return m_renderPtr.get();
}

void PicPlayer::StartControllerThread()
{
    m_ctrlDelPtr = std::make_shared<PicPlayerCtrlDelegate>();
    m_tPicCtrlThread = std::thread(std::bind(&PicPlayer::PicDataThreadProc, this));
    m_ctrlDelPtr->WaitFirstIdleEvent();
}

void PicPlayer::StopControllerThread()
{
    if (m_ctrlDelPtr) {
        m_ctrlDelPtr->Quit();
    }
    if (m_tPicCtrlThread.joinable()) {
        m_tPicCtrlThread.join();
    }
}

void PicPlayer::RenderThreadProc()
{
#ifdef __APPLE__
    // macOS 下窗口必须在 registerWindow 之后拿到父窗口句柄再创建，否则会直接失败。
    while (!m_bStop && GetWid() == 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    if (m_bStop) {
        m_renderThreadExited.store(true);
        return;
    }
#endif

    if (nullptr == m_guiPtr) {
        m_guiPtr = PicPlayerGui::Create(GetWid());
    }
    if (!m_guiPtr)
    {
        m_renderThreadExited.store(true);
        return;
    }
    m_guiPtr->SetIRenderFactory(this);
    m_guiPtr->RunRendLoop();
    m_renderThreadExited.store(true);
}

void PicPlayer::PicDataThreadProc()
{
    if (m_ctrlDelPtr) {
        m_ctrlDelPtr->RunEventLoop();
    }
}

