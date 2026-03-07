#include "PicPlayerCtrlBase.h"
#include "../NodeDataDef/NodesDataForDraw.h"
#include "../PicPlayerDataDef.h"

PicPlayerCtrlBase::PicPlayerCtrlBase()
{
    m_renderNodesPtr = std::make_unique<RenderNodesData>();
}

PicPlayerCtrlBase::~PicPlayerCtrlBase()
{

}

void PicPlayerCtrlBase::SetCallback(PlayerMsgCallback callback, void* pUser)
{
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
    std::string showid = showPicId;
    m_callback(0, (int)Callback_ShowPicId, (void*)showid.c_str(), m_pUser);
}

