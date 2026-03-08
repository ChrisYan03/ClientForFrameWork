#include "PicPlayerCtrlDelegate.h"
#include "PicPlayerCtrlBase.h"
#include "../NodeDataDef/NodesDataForDraw.h"
#include "PicPlayerLog.h"

PicPlayerCtrlDelegate::PicPlayerCtrlDelegate()
    : m_firstIdlePassed(false)
    , m_playerBasePtr(nullptr)
{
    CreatePlayerController();
}

PicPlayerCtrlDelegate::~PicPlayerCtrlDelegate()
{

}

void PicPlayerCtrlDelegate::SetPicCallbackByDelegate(int handle, PlayerMsgCallback callback, void* pUser)
{
    if (m_playerBasePtr) {
        m_playerBasePtr->SetCallback(handle, callback, pUser);
    }
}

void PicPlayerCtrlDelegate::InputPicData(int type, void* showData)
{
    if (showData == nullptr) {
        return;
    }
    if (type == 1) {
        PicShowInfo* originalData = static_cast<PicShowInfo*>(showData);
        PicShowInfo* copiedData = new PicShowInfo(*originalData); // 使用拷贝构造函数进行深拷贝
        std::shared_ptr<PicShowInfo> sharedShowData(copiedData, [](PicShowInfo* p) {
            if(p) {
                delete p; 
            }
        });
        m_loop.asyncInvokeAny([this, sharedShowData](){
            m_playerBasePtr->InputPicData(sharedShowData);
        });
    }
}

void PicPlayerCtrlDelegate::InputFaceRecogResult(void* recogResult)
{
    if (recogResult == nullptr) {
        return;
    }

    FaceDetectionResult* originalResult = static_cast<FaceDetectionResult*>(recogResult);
    FaceDetectionResult* copiedResult = new FaceDetectionResult(*originalResult); // Use copy constructor for deep copy
    std::shared_ptr<FaceDetectionResult> sharedRecogResult(copiedResult, [](FaceDetectionResult* p) {
        if(p) {
            delete p; 
        }
    });
    m_loop.asyncInvokeAny([this, sharedRecogResult](){
        m_playerBasePtr->InputFaceRecogResult(sharedRecogResult);
    });
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
    LOG_DEBUG("OnRenderComCallback: RenderType={}", cmd->RenderType());
    if(cmd->RenderType() == (int)NodesType::PicChangeType) {
        PicShowNow* curPtr = static_cast<PicShowNow*>(cmd);
        if (curPtr) {
            std::string showPicId = curPtr->picId;
            LOG_DEBUG("OnRenderComCallback: PicChangeType, picId={}", showPicId);
            m_loop.asyncInvokeAny([this, showPicId](){
                LOG_DEBUG("ShowPicCallback invoked for picId: {}", showPicId);
                m_playerBasePtr->ShowPicCallback(showPicId);
            });
        }
    }
    else if (cmd->RenderType() == (int)NodesType::PicRemoveType) {
        LOG_DEBUG("OnRenderComCallback: PicRemoveType");
    }
}

