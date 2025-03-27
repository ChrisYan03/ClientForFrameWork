#include "PicPlayerVideoRender.h"
#include "PicPlayerMovieByScene.h"

PicPlayerVideoRender::PicPlayerVideoRender(int cacheNum)
    : PicPlayerRender()
    , m_cacheNum(cacheNum)
{

}

PicPlayerVideoRender::~PicPlayerVideoRender()
{

}

void PicPlayerVideoRender::InitScene(const ImRect& rc)
{
    m_playScene = std::make_unique<PicPlayerMovieByScene>(rc, m_cacheNum);
    m_playScene->SetRenderSync(GetSynchronizer());
}

void PicPlayerVideoRender::InitFramerate(float frame)
{
    if (m_playScene){
        m_playScene->SetCurFramerate(frame);
    }
}
