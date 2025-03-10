#include "NodesData.h"

RenderNodesData::RenderNodesData()
    : m_dirty(false)
{

}

RenderNodesData::~RenderNodesData()
{

}

void RenderNodesData::AppendComData(std::unique_ptr<RenderComData>&& dataPtr)
{
    RenderComData* pData = dataPtr.get();
    m_dataList.remove_if([pData](const std::unique_ptr<RenderComData>& cmd){
        return cmd->IsSame(pData);
    });
    m_dataList.emplace_back(std::forward<std::unique_ptr<RenderComData>>(dataPtr));
    m_dirty = true;
}

std::list<std::unique_ptr<RenderComData>>& RenderNodesData::GetComDataList()
{
    return m_dataList;
}

void RenderNodesData::ClearCacheComData()
{
    m_dataList.clear();
    m_dirty = false;
}
