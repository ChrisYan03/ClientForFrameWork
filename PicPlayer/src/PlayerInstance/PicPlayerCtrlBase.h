#ifndef PICPLAYERCTRLBASE_H
#define PICPLAYERCTRLBASE_H

#include "PicPlayerRenderSync.h"
#include "../PicPlayerDataDef.h"
#include "../NodeDataDef/NodesData.h"

class PicPlayerCtrlBase
{
public:
    explicit PicPlayerCtrlBase();
    ~PicPlayerCtrlBase();

    void SetRenderSync(std::shared_ptr<PicPlayerRenderSync> syncPtr);
    void CheckSyncRenderData();
    void InputPicData(PicShowInfo* showData);

protected:
    std::shared_ptr<PicPlayerRenderSync> m_syncPtr;
    std::unique_ptr<RenderNodesData> m_renderNodesPtr;
};

#endif // PICPLAYERCTRLBASE_H
