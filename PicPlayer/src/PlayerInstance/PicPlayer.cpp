#include "PicPlayer.h"


PicPlayer::PicPlayer(int cacheNum)
    :m_cacheNum(cacheNum)
    ,m_handle(-1)
{

}

PicPlayer::~PicPlayer()
{

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
