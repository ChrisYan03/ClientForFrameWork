#include "PicPlayerCtrlDelegate.h"
#include "PicPlayerCtrlBase.h"

PicPlayerCtrlDelegate::PicPlayerCtrlDelegate()
    : m_firstIdlePassed(false)
    , m_playerBasePtr(nullptr)
{
    CreatePlayerController();
}

PicPlayerCtrlDelegate::~PicPlayerCtrlDelegate()
{

}

void PicPlayerCtrlDelegate::SetPicCallbackByDelegate(PlayerMsgCallback callback, void* pUser)
{

}

void PicPlayerCtrlDelegate::Quit()
{
    m_loop.quit();
}

void PicPlayerCtrlDelegate::RunEventLoop()
{
    m_loop.idleRun([this]() {
        if (!m_firstIdlePassed) {
            m_firstIdleCondition.notify_all();
            m_firstIdlePassed = true;
        }
        //m_playerBasePtr->
    });
    m_loop.exec();
}

void PicPlayerCtrlDelegate::WaitFirstIdleEvent()
{
    if (m_firstIdlePassed) {
        return;
    }
    std::unique_lock<std::mutex> lock(m_firstIdleMutex);
    m_firstIdleCondition.wait(lock);
}

void PicPlayerCtrlDelegate::SetRenderSync(std::shared_ptr<PicPlayerRenderSync> syncPtr)
{
    if (syncPtr) {
        m_loop.asyncInvokeAny([syncPtr, this](){
            m_playerBasePtr->SetRenderSync(syncPtr);
            //syncPtr->RenderComCallback(std::bind(&PicPlayerCtrlDelegate::OnRenderComCallback, this, std::placeholders::_1));
        });
    }
}

void PicPlayerCtrlDelegate::CreatePlayerController()
{
    if (nullptr == m_playerBasePtr) {
        m_playerBasePtr = std::make_shared<PicPlayerCtrlBase>();
    }
}

void PicPlayerCtrlDelegate::OnRenderComCallback(RenderComData* cmd)
{

}
