#ifndef PICPLAYER_H
#define PICPLAYER_H

#include <thread>
#include "PicPlayerGui.h"

class PicPlayerCtrlDelegate;

class PicPlayer : public PicPlayerGui::IRenderFactory
{
public:
    PicPlayer(int cacheNum);
    virtual ~PicPlayer();

    inline void SetWid(Window_ShowID wid) { m_wid = wid; }
    inline Window_ShowID GetWid() const { return m_wid; }
    PicPlayerGui* GetGui() const { return m_guiPtr.get(); }

    void SetHandle(int handle);
    void SetPicCallback(PlayerMsgCallback callback, void* pUser);

    bool InputPicData(int type, void* showData);
    bool StartPlayer();
    bool StopPlayer();

    bool InputFaceRecogResult(void* recogResult);

protected:
    PicPlayerRender* GetRender() const override;
    void StartControllerThread();
    void StopControllerThread();
    void RenderThreadProc();
    void PicDataThreadProc();

private:
    int m_cacheNum;
    int m_handle;

    bool m_bStop;

    Window_ShowID m_wid;

    std::thread m_tPicCtrlThread;
    std::thread m_tRenderThread;
    std::shared_ptr<PicPlayerGui> m_guiPtr;
    std::shared_ptr<PicPlayerCtrlDelegate> m_ctrlDelPtr;
    std::shared_ptr<PicPlayerRender> m_renderPtr;
};

#endif // PICPLAYER_H

