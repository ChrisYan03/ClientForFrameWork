#include "PicPlayer.h"
#include "PicPlayerVideoRender.h"
#include "PicPlayerCtrlDelegate.h"
#include <iostream>

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
    std::cout << "~PicPlayer";
    StopControllerThread();
    StopPlayer();
    std::cout << "~PicPlayer suc";
}

void PicPlayer::SetHandle(int handle)
{
    m_handle = handle;
}

void PicPlayer::SetPicCallback(PlayerMsgCallback callback, void* pUser)
{
    if (m_ctrlDelPtr) {
        m_ctrlDelPtr->SetPicCallbackByDelegate(callback, pUser);
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
    m_tRenderThread = std::thread(std::bind(&PicPlayer::RenderThreadProc, this));
    auto thread = m_tRenderThread.native_handle();
    #ifdef __APPLE__
        //设置线程优先级
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
    if (m_guiPtr) {
        m_guiPtr->Quit();
    }
    if (m_tRenderThread.joinable()) {
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
    if (nullptr == m_guiPtr) {
        m_guiPtr = PicPlayerGui::Create(GetWid());
    }
    if (!m_guiPtr)
        return;
    m_guiPtr->SetIRenderFactory(this);
    m_guiPtr->RunRendLoop();
}

void PicPlayer::PicDataThreadProc()
{
    if (m_ctrlDelPtr) {
        m_ctrlDelPtr->RunEventLoop();
    }
}
