#include "PicPlayerVideoRender.h"
#include "PicPlayerMovieByScene.h"
#include "PicPlayerImageByScene.h"

PicPlayerVideoRender::PicPlayerVideoRender(int cacheNum, PicShowType showType)
    : PicPlayerRender()
    , m_cacheNum(cacheNum)
    , m_showType(showType)
{}

PicPlayerVideoRender::~PicPlayerVideoRender() {}

void PicPlayerVideoRender::InitScene(const ImRect& rc)
{
    if (m_showType == PicShowType_Move) {
        m_playScene = std::make_unique<PicPlayerMovieByScene>(rc, m_cacheNum);
    } else {
        m_playScene = std::make_unique<PicPlayerImageByScene>(rc, m_cacheNum);
    }
    m_playScene->SetRenderSync(GetSynchronizer());
}

void PicPlayerVideoRender::InitFramerate(float frame)
{
    if (m_playScene){
        m_playScene->SetCurFramerate(frame);
    }
}

