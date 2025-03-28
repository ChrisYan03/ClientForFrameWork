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

    void SetCallback(PlayerMsgCallback callback, void* pUser);
    void SetRenderSync(std::shared_ptr<PicPlayerRenderSync> syncPtr);
    void CheckSyncRenderData();
    void InputPicData(PicShowInfo* showData);

    // callback
    void ShowPicCallback(const std::string& showPicId);

protected:
    PlayerMsgCallback m_callback;
    void* m_pUser;
    std::shared_ptr<PicPlayerRenderSync> m_syncPtr;
    std::unique_ptr<RenderNodesData> m_renderNodesPtr;
};

#endif // PICPLAYERCTRLBASE_H
