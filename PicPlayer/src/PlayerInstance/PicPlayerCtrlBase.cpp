#include "PicPlayerCtrlBase.h"
#include "../NodeDataDef/NodesDataForDraw.h"

PicPlayerCtrlBase::PicPlayerCtrlBase()
{
    m_renderNodesPtr = std::make_unique<RenderNodesData>();
}

PicPlayerCtrlBase::~PicPlayerCtrlBase()
{

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

void PicPlayerCtrlBase::InputPicData(PicShowInfo* showData)
{
    if (showData) {
        auto picShowPtr = std::make_unique<PicData>();
        // 使用拷贝构造函数进行深拷贝
        picShowPtr->picShowData = *showData;
        m_renderNodesPtr->AppendComData(std::move(picShowPtr));
        delete showData;
    }
}
