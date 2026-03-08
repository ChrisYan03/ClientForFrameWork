#include "PicPlayerCtrlBase.h"
#include "../NodeDataDef/NodesDataForDraw.h"
#include "../PicPlayerDataDef.h"
#include "PicPlayerLog.h"

PicPlayerCtrlBase::PicPlayerCtrlBase()
    : m_handle(0)
{
    m_renderNodesPtr = std::make_unique<RenderNodesData>();
}

PicPlayerCtrlBase::~PicPlayerCtrlBase()
{

}

void PicPlayerCtrlBase::SetCallback(int handle, PlayerMsgCallback callback, void* pUser)
{
    m_handle = handle;
    m_callback = callback;
    m_pUser = pUser;
}

void PicPlayerCtrlBase::SetRenderSync(std::shared_ptr<PicPlayerRenderSync> syncPtr)
{
    m_syncPtr = syncPtr;
}

void PicPlayerCtrlBase::CheckSyncRenderData()
{
    if (m_renderNodesPtr && m_syncPtr) {
        if (m_syncPtr->SyncRenderNodesData(m_renderNodesPtr.get())) {
            m_renderNodesPtr->ClearCacheComData();
        }
    }
}

void PicPlayerCtrlBase::InputPicData(std::shared_ptr<PicShowInfo> showData)
{
    if (showData) {
        auto picShowPtr = std::make_unique<PicData>();
        picShowPtr->picShowData = showData;
        m_renderNodesPtr->AppendComData(std::move(picShowPtr));
    }
}

void PicPlayerCtrlBase::InputFaceRecogResult(std::shared_ptr<FaceDetectionResult> recogResult)
{
    if (recogResult) {
        auto recogPtr = std::make_unique<FaceRecogData>();
        recogPtr->picDetectionResult = recogResult;
        m_renderNodesPtr->AppendComData(std::move(recogPtr));
    }
}

void PicPlayerCtrlBase::ShowPicCallback(const std::string& showPicId)
{
    LOG_DEBUG("ShowPicCallback: handle={}, showPicId={}", m_handle, showPicId);
    if (m_callback) {
        std::string showid = showPicId;
        m_callback(m_handle, (int)Callback_ShowPicId, (void*)showid.c_str(), m_pUser);
    } else {
        LOG_WARN("ShowPicCallback: m_callback is null!");
    }
}

