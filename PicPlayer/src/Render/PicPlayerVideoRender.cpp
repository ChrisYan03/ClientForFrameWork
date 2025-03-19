#include "PicPlayerVideoRender.h"
#include "PicPlayerMovieByScene.h"

PicPlayerVideoRender::PicPlayerVideoRender(int cacheNum)
    :m_cacheNum(cacheNum)
{

}

PicPlayerVideoRender::~PicPlayerVideoRender()
{

}

void PicPlayerVideoRender::InitScene(const ImRect& rc)
{
    m_playScene = std::unique_ptr<PicPlayerScene>(new PicPlayerMovieByScene(rc, m_cacheNum));
}

void PicPlayerVideoRender::InitFramerate(float frame)
{
    if (m_playScene){
        m_playScene->SetCurFramerate(frame);
    }
}
