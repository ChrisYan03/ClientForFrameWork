#include "PicPlayerRenderSync.h"
#include <condition_variable>

PicPlayerRenderSync::PicPlayerRenderSync()
    : m_enable(false)
    , m_waitingUpdate(false)
    , m_curNoteData(nullptr)
{

}

PicPlayerRenderSync::~PicPlayerRenderSync()
{

}

bool PicPlayerRenderSync::SyncRenderNodesData(RenderNodesData* data)
{
    if (!m_enable)
        return false;
    if (data->Dirty()) {
        // 多线程重入保护
		std::lock_guard<std::mutex> lock(m_multiSyncMutex);
        m_curNoteData = data;
        m_waitingUpdate = true;
        bool waitRes = waitUpdateFinished();
        m_waitingUpdate = false;
        return waitRes;
    }
    return true;
}

RenderNodesData* PicPlayerRenderSync::BeginSync()
{
    if (!m_enable)
        return nullptr;
    if (m_waitingUpdate) {
        if (m_curNoteData == nullptr)
            UpdateFinishedWakeUp();
        return m_curNoteData;
    }
    return nullptr;
}

void PicPlayerRenderSync::EndSync(RenderNodesData* data)
{
    if (!data)
        return;
    m_curNoteData = nullptr;
    UpdateFinishedWakeUp();
}

void PicPlayerRenderSync::SetEnable(bool enable)
{
    m_enable = enable;
    if (!enable) {
        m_curNoteData = nullptr;
        UpdateFinishedWakeUp();
    }
}

void PicPlayerRenderSync::SetRenderComCallback(std::function<void(RenderComData*)>&& func)
{
    m_renderCallback = func;
}

void PicPlayerRenderSync::RenderComCallback(RenderComData* cmd)
{
    if (m_renderCallback)
        m_renderCallback(cmd);
}

bool PicPlayerRenderSync::waitUpdateFinished()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_condition.wait(lock);
    return true;
}

void PicPlayerRenderSync::UpdateFinishedWakeUp()
{
    m_condition.notify_one();
}

