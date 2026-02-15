#ifndef PICPLAYERCTRLDELEGATE_H
#define PICPLAYERCTRLDELEGATE_H

#include "../PicPlayerDataDef.h"
#include "../NodeDataDef/NodesData.h"
#include "NodesData.h"
#include "EventLoop.h"
#include <mutex>

class PicPlayerCtrlBase;
class PicPlayerRenderSync;

class PicPlayerCtrlDelegate
{
public:
    PicPlayerCtrlDelegate();
    ~PicPlayerCtrlDelegate();

    void SetPicCallbackByDelegate(PlayerMsgCallback callback, void* pUser);
    void InputPicData(int type, void* showData);
    void InputFaceRecogResult(void* recogResult);

    void Quit();
    void RunEventLoop();
    void WaitFirstIdleEvent();
    void SetRenderSync(std::shared_ptr<PicPlayerRenderSync> syncPtr);

protected:
    void CreatePlayerController();
    void OnRenderComCallback(RenderComData* cmd);

private:
    std::shared_ptr<PicPlayerCtrlBase> m_playerBasePtr;
    std::condition_variable m_firstIdleCondition;
    std::mutex m_firstIdleMutex;
    std::atomic_bool m_firstIdlePassed;
    EventLoop m_loop;
};

#endif // PICPLAYERCTRLDELEGATE_H

