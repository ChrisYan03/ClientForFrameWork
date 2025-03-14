#include "PicPlayer.h"
#include "PicPlayerVideoRender.h"
#include "PicPlayerCtrlDelegate.h"

PicPlayer::PicPlayer(int cacheNum)
    : m_cacheNum(cacheNum)
    , m_handle(-1)
    , m_wid(0)
    , m_guiPtr(nullptr)
    , m_ctrlDelPtr(std::make_shared<PicPlayerCtrlDelegate>())
    , m_renderPtr(std::make_shared<PicPlayerVideoRender>(cacheNum))
{
    StartControllerThread();
}

PicPlayer::~PicPlayer()
{
    StopControllerThread();
    StopPlayer();
}

void PicPlayer::SetHandle(int handle)
{
    m_handle = handle;
}

void PicPlayer::SetPicCallback(PlayerMsgCallback callback, void* pUser)
{

}

bool PicPlayer::InputPicData(int type, void* showData)
{
    return true;
}

bool PicPlayer::StartPlayer()
{
    m_guiPtr = std::make_shared<PicPlayerGui>();
    m_guiPtr->Create(GetWid());
    return true;
}

bool PicPlayer::StopPlayer()
{
    return true;
}

PicPlayerRender* PicPlayer::GetRender() const
{
    return m_renderPtr.get();
}

void PicPlayer::StartControllerThread()
{

}

void PicPlayer::StopControllerThread()
{

}

void PicPlayer::RenderThreadProc()
{

}

void PicPlayer::PicDataThreadProc()
{

}
