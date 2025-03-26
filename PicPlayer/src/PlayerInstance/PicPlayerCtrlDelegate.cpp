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

void PicPlayerCtrlDelegate::InputPicData(int type, void* showData)
{
    if (showData == nullptr) {
        return;
    }
    if (type == 1) {
        //
        auto curShowData = (PicShowInfo*)showData;
        PicShowInfo* showDataPtr = (PicShowInfo*)malloc(sizeof(PicShowInfo));
        *showDataPtr = *curShowData;
        showDataPtr->imageRgbaData = (char*)malloc(showDataPtr->imageRgbaLen);
        memcpy(showDataPtr->imageRgbaData, curShowData->imageRgbaData, curShowData->imageRgbaLen);
        m_loop.asyncInvokeAny([this, showDataPtr](){
            m_playerBasePtr->InputPicData(showDataPtr);
        });
    }
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
        m_playerBasePtr->CheckSyncRenderData();
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
            syncPtr->SetRenderComCallback(std::bind(&PicPlayerCtrlDelegate::OnRenderComCallback, this, std::placeholders::_1));
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
