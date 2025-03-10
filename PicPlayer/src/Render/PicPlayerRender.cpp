#include "PicPlayerRender.h"
#include "imgui_impl_opengl3_loader.h"

PicPlayerRender::PicPlayerRender()
    : m_renderSync(std::make_shared<PicPlayerRenderSync>())
{

}

PicPlayerRender::~PicPlayerRender()
{

}

void PicPlayerRender::PlayRender()
{
    //同步数据
    RenderNodesData* pData = m_renderSync->BeginSync();
    if (pData) {
        UpdateRenderNodesData(pData);
        m_renderSync->EndSync(pData);
    }

    if (m_playScene) {

    }
}

void PicPlayerRender::ClearRenderCache()
{
    if (m_playScene) {

    }
}

void PicPlayerRender::UpdateRenderNodesData(RenderNodesData* data)
{
    if (m_playScene) {

    }
}

std::shared_ptr<PicPlayerRenderSync> PicPlayerRender::GetSynchronizer() const
{
    return m_renderSync;
}

void PicPlayerRender::UpdateViewport(int width, int height)
{
    if (m_playScene) {

    }
}

PicPlayerScence* PicPlayerRender::GetScene() const
{
    return m_playScene.get();
}
