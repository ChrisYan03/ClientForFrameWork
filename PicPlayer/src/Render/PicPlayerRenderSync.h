#ifndef PICPLAYERRENDERSYNC_H
#define PICPLAYERRENDERSYNC_H

#include "../NodeDataDef/NodesData.h"
#include <mutex>

class PicPlayerRenderSync
{
public:
    PicPlayerRenderSync();
    ~PicPlayerRenderSync();

    bool SyncRenderNodesData(RenderNodesData* data);
    RenderNodesData* BeginSync();
    void EndSync(RenderNodesData* data);
    void SetEnable(bool enable);
    void SetRenderComCallback(std::function<void(RenderComData*)>&& func);
    void RenderComCallback(RenderComData* cmd);

protected:
    bool waitUpdateFinished();
    void UpdateFinishedWakeUp();

private:
    std::atomic_bool m_enable;
    std::atomic_bool m_waitingUpdate;
    std::condition_variable m_condition;
    std::mutex m_mutex;
    std::mutex m_multiSyncMutex;

    RenderNodesData* m_curNoteData;
    std::function<void(RenderComData*)> m_renderCallback;
};

#endif // PICPLAYERRENDERSYNC_H
